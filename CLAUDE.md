# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

This is a ZBOSS Zigbee protocol stack implementation for Linux (NCP host mode) with a custom wrapper library for application-level testing. The primary component is `application/zboss_wrapper`, which implements a shared library (`.so` file) that will be called by a .NET application to provide Zigbee testing functionality.

The project is based on the Nordic Semiconductor/DSR ZBOSS stack and is configured for Linux NSNG platform with NCP (Network Co-Processor) host feature set.

## Build System

The project uses a Makefile-based build system with several configuration files:

- `Platform_config` - Defines platform (`linux-nsng`) and feature set (`ncp-host`)
- `Options` - Contains compiler flags, defines, and build rules
- Build type: Release mode (`-O3`) with trace enabled

### Building the Project

From repository root:
```bash
make              # Build all applications
make clean        # Clean build artifacts
make rebuild      # Clean and rebuild
make depend       # Generate dependencies
```

### Building the zboss_wrapper Shared Library

From `application/zboss_wrapper`:
```bash
make                # Build executable
make shared         # Clean and build .so file
make shared_noclean # Build .so without cleaning
```

The shared library is built with `-fPIC` flag and outputs to `zboss_wrapper.so`.

## Architecture

### Core Workflow

The wrapper library follows a specific lifecycle that must be respected:

1. **Initialization** - `init()` called with callbacks and start parameters
2. **Device Context Registration** - `register_device_context()` configures endpoints/clusters
3. **Start** - `start()` begins ZBOSS main loop (runs continuously)
4. **Runtime Operations** - Other functions (send commands, read/write attributes, etc.) can be called while running
5. **Stop** - `stop()` shuts down the stack

### Key Components

**zboss_wrapper.c/h** - Main wrapper implementation with:
- Initialization and lifecycle management (init/start/stop)
- ZDO operations (permit joining, leave requests, network address requests, etc.)
- ZCL operations (read/write attributes, discover attributes, send commands)
- Binding operations (bind/unbind requests, binding table queries)
- Reporting configuration (configure/read reporting)
- Install code management (add/list/remove install codes)
- Signal and callback handling

**types.h** - Type definitions for:
- Request/response structures for all operations
- Callback function type definitions
- Data structures matching ZBOSS internal types

### Callback System

The wrapper uses a callback structure (`callbacks_t`) passed during initialization that contains function pointers for:
- Asynchronous operation responses (e.g., `mgmt_lqi_req`, `attr_read`, `bind_req`)
- Stack events (e.g., `zboss_start`, `device_annce`, `leave_signal`)
- Attribute reports (`attribute_report`)
- Command reception (`cmd_receive`)
- Error handling (`error`)

All operations are asynchronous - functions schedule ZBOSS callbacks and results are returned via the callback structure.

### ZBOSS Integration

The wrapper integrates with ZBOSS stack via:
- **Buffer Management** - Uses `zb_buf_get_out_delayed()` and similar functions for async operations
- **Scheduler** - `ZB_SCHEDULE_APP_CALLBACK()` for scheduling operations in ZBOSS context
- **Signal Handler** - `zboss_signal_handler()` processes stack events (network steering, device announce, leave, etc.)
- **Device Handler** - `device_handler()` processes incoming ZCL commands and responses
- **Main Loop** - `zboss_main_loop()` runs continuously after start

## Directory Structure

- `application/` - Application implementations
  - `zboss_wrapper/` - **Primary component**: Shared library for .NET interop
  - `custom_cluster/` - Custom cluster examples (coordinator/router)
  - `light_sample/` - ZCL light sample applications
  - `simple_gw/` - Simple gateway implementation
- `src/` - ZBOSS stack source code (aps, commissioning, zcl, zdo, etc.)
- `include/` - Header files (ha, zcl, osif, ncp, se)
- `lib/` - Compiled ZBOSS libraries (`libzboss.a`, `libzboss.ed.a`)
- `ncp_fw/` - NCP firmware files

## Configuration

**Platform**: `linux-nsng` (Linux with Nordic Semiconductor Next Generation)
**Feature Set**: `ncp-host` (NCP host mode - Zigbee stack runs on separate chip)
**Trace Level**: 3 with mask `0xFF18FFFC`
**Build Mode**: Release (`-O3`)

Key defines:
- `ZB_CONFIG_LINUX_NSNG` - Platform configuration
- `ZB_TRACE_LEVEL=3` - Trace verbosity
- `ZB_TRACE_MASK=0xFF18FFFC` - Trace categories

## Working with zboss_wrapper

When modifying the wrapper:

1. **Maintain Async Pattern** - All operations use ZBOSS scheduler and callbacks
2. **Buffer Management** - Always request buffers via `zb_buf_get_out_delayed()` and free them with `zb_buf_free()`
3. **Parameter Passing** - Global variables store request parameters before scheduling callbacks
4. **Response Handling** - Parse ZBOSS responses in callback functions and invoke user callbacks
5. **Lifecycle Ordering** - Ensure init → register_device_context → start → operations sequence

### Test Program

`test_so.c` demonstrates shared library usage:
- Load library with `dlopen()`
- Get function pointers via `dlsym()`
- Call init with callbacks and parameters
- Call start to begin operation

## Common Operations

### Adding New ZCL Commands

1. Define request/response types in `types.h`
2. Add callback typedef to `callbacks_t` structure
3. Declare global variable for request parameters
4. Implement callback function that parses response
5. Implement request preparation function (scheduled callback)
6. Implement public API function that schedules the request

### Debugging

Trace logs output to `/var/log/zboss_wrapper` (configured in `ZB_INIT()` call). Adjust `ZB_TRACE_LEVEL` and `ZB_TRACE_MASK` in `Options` file for different verbosity.

## Dependencies

- **pthread** - Required for ZBOSS operation
- **ZBOSS libraries** - `lib/libzboss.a` or `lib/libzboss.ed.a` (for end devices)
- **NCP device** - Physical Zigbee NCP connected via serial (default: `/dev/ttyACM0`)

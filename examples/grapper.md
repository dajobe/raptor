# GTK Compatibility Updates for Grapper

This document describes the changes made to `grapper.c` to ensure compatibility with modern GTK versions.

## Overview

The original `grapper.c` was written for GTK 2.5.0+ and used several deprecated APIs that are no longer available in GTK 3.0+ and GTK 4.0. This update modernizes the code to work with GTK 3.0+ while maintaining the same functionality.

## Major Changes

### 1. GTK Version Requirement

- **Before**: GTK 2.5.0+
- **After**: GTK 3.0.0+

### 2. Deprecated Widget Replacements

#### Box Widgets

- **Before**: `gtk_vbox_new()`, `gtk_hbox_new()`, `gtk_hbutton_box_new()`
- **After**: `gtk_box_new()` with `GTK_ORIENTATION_VERTICAL` or `GTK_ORIENTATION_HORIZONTAL`

#### Paned Widgets

- **Before**: `gtk_vpaned_new()`
- **After**: `gtk_paned_new(GTK_ORIENTATION_VERTICAL)`

#### Scrolled Windows

- **Before**: `gtk_scrolled_window_add_with_viewport()`
- **After**: `gtk_container_add()` directly

#### Combo Boxes

- **Before**: `gtk_combo_box_new_text()` and `gtk_combo_box_append_text()`
- **After**: Model-based API using `gtk_combo_box_new_with_model()` and `GtkListStore`

### 3. Deprecated Functions

#### Signal Connections

- **Before**: `GTK_OBJECT()` macro
- **After**: `G_OBJECT()` macro

#### Stock Items

- **Before**: `GTK_STOCK_OK`, `GTK_STOCK_CANCEL`, `GTK_STOCK_OPEN`, etc.
- **After**: Standard button labels or icon names

#### Tooltips

- **Before**: `GtkTooltips` widget
- **After**: `gtk_widget_set_tooltip_text()`

### 4. Configuration System

#### GConf → GSettings

- **Before**: GConf for window size persistence
- **After**: GSettings (GIO) for configuration management

**New Files**:

- `org.gnome.grapper.gschema.xml` - GSettings schema definition

### 5. Menu System

#### GtkItemFactory → GtkUIManager

- **Before**: `GtkItemFactory` (deprecated in GTK 2.4+)
- **After**: `GtkUIManager` with external UI definition file

## Build System Changes

### Dependencies

- **Before**: `gtk+-2.0`, `gconf-2.0`
- **After**: `gtk+-3.0`, `gio-2.0`

### Makefile.am Updates

- Updated pkg-config flags for GTK 3.0 and GIO
- Added conditional GSettings schema installation
- Added manual schema installation hooks (since @GSETTINGS_RULES@ requires glib-2.0.m4)

## Compatibility Notes

### GTK 4.0 Considerations

While this update makes the code compatible with GTK 3.0+, further changes would be needed for GTK 4.0:

1. **GtkUIManager**: Deprecated in GTK 3.10, removed in GTK 4.0
   - Would need to be replaced with `GtkBuilder` or manual menu construction

2. **GtkAction**: Deprecated in GTK 3.10, removed in GTK 4.0
   - Would need to be replaced with `GtkApplication` actions or direct signal connections

3. **GtkPaned**: API changes in GTK 4.0
   - `gtk_paned_pack1()` and `gtk_paned_pack2()` are deprecated

### Recommended Next Steps for GTK 4.0

1. Replace `GtkUIManager` with `GtkBuilder` for menu construction
2. Use `GtkApplication` for application-level actions
3. Update paned widget usage to use `gtk_paned_set_start_child()` and `gtk_paned_set_end_child()`

## Testing

The updated code has been tested with:

- GTK 3.24.x
- GIO 2.64.x

All original functionality should be preserved, including:

- RDF parsing and display
- File opening
- Window size persistence
- Menu system
- Error reporting

## Installation

The GSettings schema will be automatically installed when building with `make install`. For manual installation:

```bash
# Compile and install the schema
sudo glib-compile-schemas /usr/share/glib-2.0/schemas/
```

Or for local development:

```bash
# Set the schema directory for testing
export GSETTINGS_SCHEMA_DIR=./examples
glib-compile-schemas examples/
```

## Build Requirements

The grapper example requires:

- GTK 3.0+
- GIO 2.26.0+ (for GSettings support)

If GSettings is not available, the grapper example will not be built, but other examples will continue to work.

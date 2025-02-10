# Keyboard Shorcuts

[Versione in italiano](SHORTCUTS_it.md)

## View status bar

When you have keyboard focus on a view like Circuit View or Panel view,
you can use `Shift + Z` to toggle toolbar visibility.

This way you can save some screen space when many views are visible at same time.

## View zoom

You can zoom in/out by pressing `Ctrl` and rotate mouse wheel.
Or you can enter zoom level in view's status bar.

## Render View to SVG

Press `Ctrl+R` on an active view, a file dialog will appear for chosing destination SVG file.

## Properties dialog

### Object fields

To remove an object from a field, delete text and press `Enter`.

### Number fields

Number fields can be edited with mouse wheel if you pass with cursor over them.
If they are inside a scroll area, you must first click to get focus on them and then scroll to change number.

### Nodes Tab

In object Properties dialog, the "Nodes" tab shows a list of all nodes referencing the object.
Double clicking on an item will show (or raise if already open) circuit view and center it on selected node.

If `Shift` is held while clicking, a new view is opened regardless if other views exist for same circuit.
If `Alt` is held while clicking, the zoom level in the view will not be raised if below 100%.

## Editing modes

### Default

You can:
- Add/delete nodes in circuits.
- Rotate nodes (`Right click` clockwise or `Shift + Right click` counterclockwise).
- Flip nodes (`Ctrl + Right click` for deviator contact nodes, simple nodes and combinator relay power nodes).
- Add/Remove cables.

You can move only one node at a time by dragging it. You cannot move cables.
To place the node over an existing cable, hold `Shift` while releasing mouse. Cable will be splitted into 2 segments.

Double click on a node/cable to show editing dialog.

This is default mode. If you are in another mode, press `Esc` to return to Default mode.

### Cable editing

When in cable editing you create the path for a cable by:
- `Left click` on a cell edge to start the cable. A little green line will appear.
- `Left click` on other cells to make cable go horizontal or vertical from its last point.
- `Right click` on cell edge to finish a cable. It will become red.
- Press `Enter` to confirm or `Esc` to cancel.

Use `Ctrl + Z` during editing to undo last cable segment.

When in Default mode you can enter Cable Editing by:
- `Shift + Left click` on a existing cable.
- `Double click` on an existing cable, then `Edit Path` in the dialog.
- Press `Cable` on edit toolbar to create a new cable.
- Press `C` shortcut to create a new cable.

### Selection mode

In this mode you can select multiple nodes and cables to delete them, move them, or copy paste them.
You cannot rotate nodes or edit cable path.

From default mode you can enter selection by pressing `Shift + S` inside a Circuit View.

You can exit selection mode by pressing `Esc`.

To select items you can:
- `Left click` on a node/cable to select it, previously selected items will be unselected.
- Drag mouse from an empty area, all items touched by the rubber band will be selected. All previously selected items will be unselected.
- `Ctrl + Left click` on a node/cable to add/remove it from current selection.
- `Left click` on an empty area to clear current selection.
- `Ctrl + A` to select everything in current view.
- `Shift + Ctrl + A` to invert selection.

To copy current selection press `Ctrl + C`.
To paste items into a view, press `Ctrl + V`

# Items shortcuts

You can see this by hovering an item in edit toolbar.
Node type name and its shourcut are shown in the tooltip.

## Basic
- `P` for a new power source
- `F` for a new simple node
- `C` for a new cable

## Contacts
- `B` for Button contact node
- `K` for a new lever contact
- `R` for a new relay contact
- `X` for screen relay contact A/B

## Powering nodes
- `E` for a new relay power node
- `L` for a new light bulb power node
- `M` for a new electromagnet power node
- `V` for screen relay power node
- `U` for sound node

## Misc
- `D` for diode node
- `I` for polarity inversion node
- `O` for On/Off switch node
- `Q` for bifilarizator node

## Remote circuit
- `T` for remote circuit connection node


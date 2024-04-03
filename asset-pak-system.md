# Asset system

## Why:
* Loading many small files are slow
* Deleting many small things are slow
* Don't want to track many small things that may or may not be loaded

## Design
* Assets lives in PAKs or WADs to a Block
* Blocks are loaded on a stack
* Asset in blocks can only reference stacks loaded earlier

## Possible setup:
* UI assets
* Common game assets
* Level theme assets (snow player)
* Actual level assets

This allows us to easily unload assets for a specific level, loading becomes easier since we know exactly what will be loaded and can easily add a progress bar.

* Streaming can be solved by temporaily allowing a branch in the stack and do the actual pop/push when switching area.
* Asset handles need to reference the block and index and block revision. No revision on actual asset since it's part of the block.

## Issues
* Packing can be a problem during dev.
  This could be solved by having a slower vfs/filesystem pak implementation and switch (with DI?) to regular pak for dist builds
* Can be hard to author data.
  This could be solved by placing assets in special folders and have a vfs like build step where one could author the "stack tree"
* No paths can be specified in code.
  This could both be a good thing since assets should be provided(perhaps via a something similar to typescript interfaces), this would also be useful to make validating data easier. This could also be solved by having a pre-compilation step that replaces paths with actual references...

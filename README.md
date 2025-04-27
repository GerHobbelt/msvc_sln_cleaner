# MSVC2022+ Solution (`*.sln`) File Cleaner (Utility) 

This tool cleans up MSVC Solution (`.sln`) files which have been patched by external tools (or by hand).

## What's the problem this solves?

Previously (before April 2025) MSVC was quite lenient when loading `.sln` files: 

1. any duplicate project entries were mentioned (alert box) but otherwise ignored/discarded, which was perferctly fine with me.
2. any `Project` slots which happened to have the same UUID mentioned in their SLN entry line (a duplication of info AFAIAC anyway) were simply assigned a new UUID (the UUID from their `.vcxproj` project file, if available), which was perferctly fine with me.

All this **stopped working** in MSVC2022 v17.13.6 and thus a lot of build/preprocessing/tooling scripts we use suddenly turned showstopper as the new MSVC release barfs a hairball (alert box) and **terminates & discards the loading process**.

This utility is a fast(-ish) patch to the toolset to ensure we produce `.sln` files which are accepted by the latest MSVC2022 IDE.

## Who's affected?

At least me, myself & I, when running the tooling (bash) shell scripts at, for example, https://github.com/search?q=repo%3AGerHobbelt%2Fmupdf+%2Fplatform%5C%2Fwin32%5C%2F.*%5B.%5Dsh%2F&type=code


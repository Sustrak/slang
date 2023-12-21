# slang-lsp

A simple LSP client for SystemVerilog

## Debug

To debug the LSP server I find Kate the most useful LSP client since it has debug options to print the
LSP messages send by the client.

To build your own version of kate please follow [this guide](https://kate-editor.org/build-it/). To build 
the `extra-cmake-modules` you will need to install as well `pip install sphinxcontrib-qthelp` which is
not installed by `kdesrc-build`.

Later you can use `run_kate.py` to launch kate with the debug options and invoke `gdb` on the LSP server.
If you have built `kate` locally do `source ~/projects/kde/build/kde/applications/kate/prefix.sh` to put 
the local `kate` in the PATH.

To build kate again, if you do any change on its source code to help you debug the LSP Server, you can do:
```shell
source ~/projects/kde/build/kde/applications/kate/prefix.sh
kdesrc-build --no-include-dependencies kate
```

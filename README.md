# typst-graphviz-plugin
A method to compile a small subset of graphviz functions to wasm for use as a typst plugin

It should contain everything needed to compile but the following tools are needed:
- Linux OS (It might work elsewhere but i didn't try it)
- python3 (for stub replacement)
- wabt (for conversion between wat/wasm)
- emcc (for compiling the plugin)

after cloning the repo the ./Build should create the wasm file (let me know if there are problems). 

# Work left to do
- Fonts are not appropriately handled and basically cause the plugin to lock up. I think this needs to be figured out, but is fairly easy to work around.

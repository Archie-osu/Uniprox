# Uniprox
A glorified call to LoadLibrary via stubbing dbgcore.dll
# How do I use this
You place Uniprox's DLL into the target application's folder (next to the executable), name it ``dbgcore.dll``, and run the app.

The Uniprox DLL takes over chain-loading every single DLL from the "uniprox" folder in the application directory.  

# Distributed Open Infrastructure for Network Computing

# TODO
* [x] Sandboxed LuaJIT execution
* [ ] Flesh out Readme
* [ ] JSON in the Lua environment
* [ ] Design protocol
* [ ] Implement worker client and supplier node
* [ ] Implement NAT forwarding
* [ ] Implement CPU, Memory limits
* [ ] Implement network bootstrapping

# Get up and running
Use `./get_dependencies.sh` to download and build LuaJIT and LuaState.
Then use `make dist/localworker` to build the current prototype.

Example usage:
```
make dist/localworker && dist/localworker examples/fibonacci.lua
```

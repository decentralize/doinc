# Decentralized Open Infrastructure for Network Computing (DOINC)

DOINC is a a decentralized system for volunteer computing without relying on trust.
A safe execution sandbox eliminates risk associated with executing work for third parties, and constructing the sandbox within the LuaJIT runtime to still enables competetive performance.
Projects are hosted by suppliers in a network that, while currently dependent on a nameserver for bootstrapping, is designed to be fully decentralized.
Workers find project suppliers by their project ID and can download cryptographically signed blueprints and work units to execute, handing the results back to the supplier upon completion.

This implementation, while incomplete, acts as a proof of concept for this vision.

# Current limitations
* Linux-only
* Flood-type network - does not scale
* No support for only working under certain conditions - like when screensaver is active
* Incomplete UDP support - possibility for packet mangling
* Not fully decentralized - uses a name server for bootstrapping
* I/O in worker blueprints is restricted to a string input and a string output

# Requirements

# Build
The project use GNU Make is tested with GCC 4.8.
Boost must be installed on the system.

Use `./get_dependencies.sh` to download and build LuaJIT, LuaState, ProtoBuf and CryptoPP into lib/.
Then use `make` to build all the binaries to dist/{nameserver, supplier, worker, localworker}

# Usage
## Localworker
The localworker binary emulates a supplier connected to a single worker note.
The supplier uses the current directory as working directory.
Execute in a directory where you have a client.lua and a server.lua (see examples).

## Supplier
To run a supplier node, the endpoint of a DOINC name server must be passed along
```sh
supplier <ip:port>
```

On startup, the supplier will log the project ID needed for workers.

## Worker
To run a worker, the endpoint of a DOINC name server must be passed along, along with the project ID
```sh
supplier <ip:port> <id>
```

# Contributing
Contributions are very welcome in the form of pull requests to this repository.

For suggestions of possible contributions, see the issues page.

# License
DOINC is licensed under the GNU Lesser General Public (LGPLv3) License - see the LICENSE file for details

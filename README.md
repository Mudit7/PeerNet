# PeerNet

A BitTorrent-style peer-to-peer file sharing system in C++, built around a central tracker and
group-scoped file sharing. Every client is simultaneously a server: it downloads chunks from
multiple peers in parallel while seeding chunks it already holds.

Written as a systems project at IIIT Hyderabad (2020).

## What it does

- **Tracker** holds all metadata — users, groups, which peers hold which files — and hands a
  requesting client a peer list. It never touches file data.
- **Clients** transfer file data directly between each other. No file bytes pass through the tracker.
- **Group-scoped sharing** — files are shared within groups, with an owner and join/leave semantics.
- **Chunked parallel download** — a file is split into 512 KB chunks; the client spawns a thread per
  available peer and pulls chunks concurrently, tracking per-chunk completion in a shared bitmap.
- **SHA-1 integrity** — each chunk is hashed on arrival and verified against the digest advertised by
  the tracker, so a corrupt or malicious peer can't silently poison a download.

## Architecture

```
          ┌─────────┐
          │ Tracker │   users, groups, file → peer-list index
          └────┬────┘   (metadata only, no file data)
       ────────┼────────
      │        │        │
   ┌──┴──┐  ┌──┴──┐  ┌──┴──┐
   │Peer │──│Peer │──│Peer │   chunk transfer, peer to peer
   └─────┘  └─────┘  └─────┘
```

Each peer runs three concurrent roles on pthreads:

| Thread | Role |
|---|---|
| `inputthread` | reads user commands from stdin without blocking transfers |
| `peerserverthread` | listens for incoming chunk requests, spawns a `seeder` per connection |
| `leecher` | one per remote peer, pulls chunks for an in-flight download |

Shared tracker state (`filePortMap`, `sizeMap`, `hashMap`, `grpMap`) is guarded by a pthread mutex,
since every client connection is serviced on its own thread.

## Integrity model

`getHash()` walks the file in 512 KB chunks and produces a SHA-1 digest per chunk, concatenated into
a per-file digest string. On download, `getChunkHash()` recomputes the hash of each received chunk
and compares it against the corresponding slice. Verification is per chunk rather than per file, so
a bad chunk is caught on arrival instead of after the whole transfer completes.

## Commands

```
create_user <user> <passwd>      login <user> <passwd>       logout
create_group <group>             join_group <group>          leave_group <group>
list_groups                      list_files
upload_file <path> <group>       download_file <file> <group> <dest>
```

## Build

Requires `g++`, OpenSSL and pthreads.

```bash
./automake.sh          # make clean && make && rm *.o
./tracker 4000         # start the tracker
./client 5000 4000     # start a peer (own port, tracker port)
```

> **Known issue:** `includes.h` line 1 is `#include </Users/mudit/stdc++.h>`, an absolute path from
> the original dev machine. Replace it with `#include <bits/stdc++.h>` (Linux/g++) or the individual
> standard headers before building elsewhere. The makefile also hardcodes
> `/usr/local/opt/openssl` for the OpenSSL include and lib paths; adjust for your install
> (`/opt/homebrew/opt/openssl` on Apple Silicon).

## Limitations

Deliberately scoped as a course project, so a few things a production tracker would need are absent:

- Peers bind to `127.0.0.1`; there's no NAT traversal or WAN support.
- Credentials are stored and compared in plaintext.
- Chunk selection is naive — peers are asked in list order, with no rarest-first or endgame strategy.
- Tracker state is in-memory only; restarting it loses all registrations.
- `getChunkHash()` returns `digest.c_str()` on a local `std::string`, a dangling pointer that happens
  to work in practice here but is undefined behaviour.

# MoonBox
MoonBox is my idea of an extensible Lua environment / "ecosystem" for creating
cross-platform applications. At the moment, it is mostly just my playground
for learning about C, Lua's C API and creating native applications in
general. The mindset of this project is to keep things reasonably lightweight,
and to write most of the code myself, even though others might do a better
job (it's also about the learning aspect).

At this point, MoonBox is far from being complete. I will keep working on
this, and maybe some time this will turn out to be a valuable project for not
just me. The project does already contain some very useful modules,
though - I especially like my interactive Lua command-line interface.

### Features / half-implemented
- Callback-based event system
- C modules for SDL2 input and basic graphics, basic terminal graphics, basic
	threads, terminal input with autocompletion and arbitrary data buffers
- Includes some Lua modules (some of which I really like) for GUIs, streams,
	ANSI terminal colours and autocompletion

### Ideas
These are (in no particular order) some big ideas I have planned for this
project. (Also includes some notes)
- Packages (zip format for easy downloading/sharing)
	- [libarchive](https://github.com/libarchive/libarchive) /
		[lua-archive](https://github.com/brimworks/lua-archive),
		[zziplib](https://github.com/gdraheim/zziplib) /
		[luazip](https://github.com/mpeterv/luazip),
		[libzip](https://github.com/nih-at/libzip)
	- Sandboxing (just like my other
		[quite similarly named project](https://github.com/Dantevg/MoonBox))
	- Permissions (writing to file system, loading other modules, etc.)
	- Capabilities (e.g. package needs display, network. Things that need to
		be available, but don't necessarily need to be permitted)
- Good back-end system (SDL2 vs something else, window output, event input,
	multiple back-ends)
- OS-like concepts (these might be far-fetched / weirdly specific / not useful)
	- Processes / threads / multiple Lua states (threads are already
		half-implemented)
	- Custom file systems? Might be nice to play around with
- Display: SDL2 (currently), OpenGL, Cairo with SDL2/OpenGL?

### Contributing
I don't expect anybody to be interested, but if you want to contribute,
please do! This has been a solo learning project since the start, so any
feedback or contribution is welcome.

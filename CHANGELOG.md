# CHANGELOG

# 0.6

- cmds: add depend command, adding item dependencies to projects ([400e4a4](https://github.com/Jxcob-R/tojo/commit/400e4a4))
- feat(graph): convert list of item dependencies to graph (DAG) ([09756aa](https://github.com/Jxcob-R/tojo/commit/09756aa))
- (tag: tojo-0.6rc1, origin/feat/item-dependency-graph, feat/item-dependency-graph) feat: print relevant dependencies of item given by the ID. ([c533074](https://github.com/Jxcob-R/tojo/commit/c533074))
- chore: clean up unnecessary code comments and outdated suggestions. ([2613c98](https://github.com/Jxcob-R/tojo/commit/2613c98))
- refactor: Use better LLVM code formatting for better consistency ([5c60b4c](https://github.com/Jxcob-R/tojo/commit/5c60b4c))
- feat: simple dependency tree printing with graph of dependencies ([9e240ed](https://github.com/Jxcob-R/tojo/commit/9e240ed))
- feat: printing item dependency graphs with list option ([e3f3ab9](https://github.com/Jxcob-R/tojo/commit/e3f3ab9))
- feat(cmds): Item dependency listings can now be specified with item code ([4125b77](https://github.com/Jxcob-R/tojo/commit/4125b77))
- fix: commands accepting codes now recognise all 7-characters of code ([5297a6f](https://github.com/Jxcob-R/tojo/commit/5297a6f))

# 0.5

- fix: add missing new line to main help ([d0db1a3](https://github.com/Jxcob-R/tojo/commit/d0db1a3))
- fix: item code generation ([8e8835c](https://github.com/Jxcob-R/tojo/commit/8e8835c))
- (release/0.4) fix(ds): string length also checked for validating item code ([ebbc3c0](https://github.com/Jxcob-R/tojo/commit/ebbc3c0))
- feat(cmds): list option for status view ([f54d460](https://github.com/Jxcob-R/tojo/commit/f54d460))
- memfix: save potential NULL reference from item code ([b7993d6](https://github.com/Jxcob-R/tojo/commit/b7993d6))
- chore: add install script ([6267709](https://github.com/Jxcob-R/tojo/commit/6267709))
- feat(cmds): add backlog command ([300b319](https://github.com/Jxcob-R/tojo/commit/300b319))
- refactor(cmds): improve code validation process cleanliness ([80ad0c1](https://github.com/Jxcob-R/tojo/commit/80ad0c1))

# 0.4

- compat: use isatty to modify pretty (coloured) print behaviour ([0b59be4](https://github.com/Jxcob-R/tojo/commit/0b59be4))
- chore: add NDEBUG flag to final build ([02e3fa7](https://github.com/Jxcob-R/tojo/commit/02e3fa7))
- feat: item codes and prefixes ([025c430](https://github.com/Jxcob-R/tojo/commit/025c430))
- refactor: clear trailing and leading whitespace in item names ([3b5707a](https://github.com/Jxcob-R/tojo/commit/3b5707a))
- feat: set defaults for subcommands ([a3819fc](https://github.com/Jxcob-R/tojo/commit/a3819fc))
- docs: update README with guide and overview ([af8f8e8](https://github.com/Jxcob-R/tojo/commit/af8f8e8))

# 0.3

- feat(cmds): Add resolve 'res' command to mark items as done ([1e263f1](https://github.com/Jxcob-R/tojo/commit/1e263f1))
- refactor: add typedef for item ID type ([d0abbe6](https://github.com/Jxcob-R/tojo/commit/d0abbe6))
- feat(cmds): add item restaging as add command flag ([6e789e0](https://github.com/Jxcob-R/tojo/commit/6e789e0))
- fix(cmds): interpret user input as base 10 correctly ([81e55e8](https://github.com/Jxcob-R/tojo/commit/81e55e8))
- docs: document command functionality in help pages ([3bd758a](https://github.com/Jxcob-R/tojo/commit/3bd758a))
- feat: order items in project with respect to their status ([a73e260](https://github.com/Jxcob-R/tojo/commit/a73e260))
- refactor(dir): abstract opening of status item file descriptors ([8721f3b](https://github.com/Jxcob-R/tojo/commit/8721f3b))

# 0.2

- refactor: store items in staging files. ([bf88f0f](https://github.com/Jxcob-R/tojo/commit/bf88f0f)) 
- refactor: fixed width item storage ([78068e3](https://github.com/Jxcob-R/tojo/commit/78068e3)) 
- feat: Enable ID indexation of items ([e697792](https://github.com/Jxcob-R/tojo/commit/e697792)) 
- feat: add support to item status with work ([f8253b0](https://github.com/Jxcob-R/tojo/commit/f8253b0)) 
- refactor(dir): store next available project item ID ([a1e0a1e](https://github.com/Jxcob-R/tojo/commit/a1e0a1e)) 
- refactor: remove deprecated macros from tojo ([fa34adb](https://github.com/Jxcob-R/tojo/commit/fa34adb)) 

# 0.1 - *Initial release*

- feat: create project, add and list task items ([036888f](https://github.com/Jxcob-R/tojo/commit/036888f))
- refactor: move project finding logic to dir ([aa8b346](https://github.com/Jxcob-R/tojo/commit/aa8b346))
- refactor(ds): remove _item temporary struct name ([448c9d3](https://github.com/Jxcob-R/tojo/commit/448c9d3))
- build(make): adopt ASAN developer flags ([c781d62](https://github.com/Jxcob-R/tojo/commit/c781d62))
- docs: update commit type suggestions ([4503d20](https://github.com/Jxcob-R/tojo/commit/a25c2e9))

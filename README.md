# tojo - Local task tracker

A simple terminal-based to-do tool that can help you track project progress
locally, separate to team-based tickets (i.e., Jira or Linear).

## Installation

Support for GNU/Linux OS's only. Support for Mac can be introduced later.

1. Clone and open project
2. Run `make final`
3. Put `tojo` binary in your path

## Simple user guide

### Item structure

The aim of this project is to provide a very minimal interface for interacting
with task items in personal and individual projects.

As such, tasks can be in 1 of three states:
- *todo*
- *in-progress*
- *done*

Interacting with task items, you will notice a few important details:

- Task ID: Unique numeric identifiers
- Task Name: *Self-explanatory*
- Task *Code*: Small string *also* representing unique items

<img src="https://i.imgur.com/fxYs9oM.png" alt="Item list" width="500"/>
*All fields displayed*

Users will appreciate the existence of the code and prefixing system, inspired
by the [jujustu project](https://github.com/jj-vcs/jj) changes history. Items
can quickly and ergonomically be referenced with respect to the previous `list`
viewing.

### Project-level commands

- `tojo init`: Creates a tojo project with a `.tojo/` data directory

- `tojo add ...`: Add items as 'todo'
    - With `-n`/`--name`: Adds new item to project with given name
    - With `-r`/`--restage`: Restage an item given its numeric ID
    - With `-c`/`--code`: Restage an item given its unique item code

- `tojo work ...`: Mark items as 'in-progress'
    - With `-i`/`--id`: Work on an itme given its numeric ID
    - With `-c`/`--code`: Work on an item given its unique item code

- `tojo res ...`: Resolve, or mark item as 'done'
    - Same options as `work`

- `tojo list`: List all items in project

## Project source structure

- `src`: Project source
    - `cmds`: Sub-commands
    - `dev-utils`: Debug tools and other dev utilities
    - `ds`: Essential data structures
    - `*`: Everything else
- `docs`: Project documentation

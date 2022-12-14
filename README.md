## SF Bastet

Simple and Fast Bastet. Made with SFML.

### Introduction

"For people who enjoy swearing at their computer, Bastet (short for Bastard Tetris) is an attractive alternative to Microsoft Word."

Have you ever thought Tetris(R) was evil because it wouldn't send you that straight "I" brick you needed in order to clear four rows at the same time? Well Tetris(R) probably isn't evil, but Bastet certainly is. >:-) Bastet stands for "bastard tetris", and is a simple ncurses-based Tetris(R) clone for Linux. Unlike normal Tetris(R), however, Bastet does not choose your next brick at random. Instead, Bastet uses a special algorithm designed to choose the worst brick possible. As you can imagine, playing Bastet can be a very frustrating experience!

### Features compared to original Bastet

* Faster algorithm, which means a more bastard depth = 2 search tree is possible in harder mode.
* Better evaluation function, which makes block look more random but still close to worst.
* Fancy pixel-styled interface with music.
* Less than 900 lines of code.
* Standard Rotation System.

### Instructions
The game is pretty self-explanatory; use the arrow keys and <space> or <enter> to browse through the menus, set the keys to anything you're comfortable with, and hit "Play!".

The default keys are as follows:
* Down: Down, or decrease level in main menu
* Left: Left
* Right: Right
* Space: Rotate tetromino clockwise, or choose difficulty in main menu
* Up: Rotate tetromino counterclockwise, or increase level in main menu
* Enter: "Hard-drop" tetromino (like pressing "Down" continuously), or start game in main menu
* p: Pause

### Normal/Harder

The game currently allows choosing between two block choosers. The second (harder) one does not show you the "next block" preview, thus achieving a higher level of bastardness.

### Installation
See the INSTALL.md file in this same directory.

### Credit

Algorithm and README from Federico Poloni

Dogica font by Roberto Mocci

Everything else by CodingJellyfish

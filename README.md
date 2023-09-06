# rfcreader 0.8
**rfcreader** the instrument for navigation on documents RFC.<br>
* the select is show txt or pdf.
* The find of keywords is no dependent case a character.

directory of settings is stored in '$HOME/.rfcreader'. At the first start, the program will create it.

Additional dependencies:
* ncurses

Options in the config file:
* dir - the directory, where RFC documents are storing.
* txt - the program for seeing the text files.
* pdf - the program for seeing the pdf files. This option can be omitted.
* reg - if set, then the find case-independent.
* fgcolor - a foreground color.
* bgcolor - a background color.
* sfgcolor - the selected color is foreground.
* sbgcolor - the selected color is background.

example
```bash
dir=/root/rfc
txt=less
pdf=
reg=on
fgcolor=white
bgcolor=black
sfgcolor=white
sbgcolor=black
```

install:<br>
```bash
$ make
# make install
```


**update**<br>
```bash
rfcreader -update [path to downloaded a rfc file archive]
```

**notice** the rfcreader is will update if set path in option dir in the settings file.

start:
`rfcreader`

# use:

input the keyword. press the key down or up to need string. Press 'Enter'. for exit from the program, press ESC.

# screenshots
**look of the program**
![](http://i.imgur.com/3NruQrQ.png)
**find by keyword and select**
![](http://i.imgur.com/VWoLOi6.png)



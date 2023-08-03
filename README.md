# SplitBMPfile

This program is used to cut Bitmap file into equal parts in commandline. Each part is saved in a new Bitmap file.
- Command-line syntax:
   + <program> <file Bmp>  [-h <parts in height>] [-w <parts in width>] 
- Example: program cutbmp.exe
- Cut 3 parts in height (save in 3 new Bitmap files):
   + cutbmp.exe d:/images/img1.bmp -h 3
- Cut 2 parts in height, 4 parts in width (save in 8 new Bitmap files):
   + cutbmp.exe d:/images/img1.bmp -h 2 -w 4

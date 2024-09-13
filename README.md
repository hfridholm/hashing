# hashing

# Overview
Imagine you want to hash something using different hashing algorithms, but you don't want to use different programs. This utility let's you do exactly that, generate checksums with a specified hash algorithm.

Every hash algorithm is implemented from scratch and is not written for performance, but to be easily understood. That is the purpose of this project, to make these hash algorithms easy to understand and replicated.

# Man page
The program has it's own simple man-page written in Troff. The man-page contains all the necessary information for using the program and it's features, just like any program.

To view the man-page, you can use the man command and specify the path to the hashing man-page in the **binary** directory.

```bash
man ./hashing.man
```

# Features

This program uses`argp`

Hit CTRL-D to input EOF symbol - to terminate the inputted message.

# Resources

[Troff Tutorial](https://www.youtube.com/watch?v=r2JrbyezhM4)
[man man-pages](https://www.man7.org/linux/man-pages/man7/man-pages.7.html)

[MD5 Wikipedia](https://en.wikipedia.org/wiki/MD5)

The website [sha256algorithm.com](https://sha256algorithm.com) was what enabled this project from the beginning, because it teaches you the sha256 algorithm very well using a step-by-step approach.

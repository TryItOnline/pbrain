# [pbrain Language Compiler](http://www.parkscomputing.com/applications/pbrain/)

The pbrain programming language is an extension I made to the [Brainf**k](http://www.muppetlabs.com/~breadbox/bf/) programming language that adds the ability to define and call procedures. If you’re wondering about the asterisks in the name of the programming language, it’s because the name is rather rude. That’s why I decided to use a name for my extension that a good bit tamer. After all, my Mom and my kids still read my site from time to time.

After making a rather simplistic [interpreter](pbrain.cpp) in C++, I decided to go ahead and make a [.NET compiler](pbrain.cs) as well, using C#.

Thanks to rdragon on the [Undernet](http://www.undernet.org/) #c++ channel and to [Daniel Cristofani](http://www.hevanet.com/cristofd/brainfuck/) for their comments and input.

## A Nice IDE

There’s a [nice integrated development environment](http://4mhz.de/bfdev.html) written by [Tim Rohlfs](http://4mhz.de/) that includes support for pbrain. I was delighted when he added support. Go download it, try it out, and let him know how much you like it.

## Some pbrain Programs

This is the pbrain program referred to in the comment header of the interpreter source.

```Pbrain
+([-])
+(-:<<[>>+<<-]>[>+<-]>)
+([-]>++++++++++[<++++>-]<++++++++>[-]++:.)
>+++>+++++>++:
>+++:
```

This pbrain program initializes a memory location to 65, the ASCII value of the letter ‘A’. It then calls a function for subsequent memory locations to copy the previous location and add one to it. Once a few cells are initialized, it prints all the cells to standard output.

```Pbrain
+([-]<[-]<[>+>+<<-]>>[<<+>>-])
+([-]>[-]+:<+)
>>+++++++++++++[<+++++>-]
++:
>++:
>++:
>++:
<<<<.>.>.>.>.
```

## The .NET Compiler

Here is the [C# source code](pbrain.cs) for the .NET compiler, which compiles both pbrain and traditional Brainf**k code. It performs some simple optimizations as well:

- A sequence of `+` and `-` instructions will be concatenated into one operation sequence, so that a sequence like `++++` will add 4 to the current memory location instead of adding 1 four times.
- Likewise, a sequence of `>` and `<` instructions will be concatenated into one operation sequence, so that a sequence like `>>>>` will move the pointer forward four locations rather than 1 location at a time.
- The null loop, `[-]`, will be compiled to set the current memory location to zero, since that is the practical effect of the loop anyway. This will allow for efficiently zeroing out a memory location whether it holds the value 1 or 10001.

## The Interpreter

Here is the [C++ source code](pbrain.cpp) for the interpreter. If you want to compile it with g++ make sure you get version 3.4 or later so it won’t choke on the `wchar_t` bits.

I don’t claim that this is a particularly efficient interpreter. I thought of it, designed it, and wrote it one evening/morning between the hours of 9:45 PM and 6:15 AM, in a single session.

This interpreter will also work with traditional Brainf**k code.

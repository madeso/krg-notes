Based on https://github.com/JACoders/OpenJK/wiki/ICARUS-Scripting

Scripts are placed on a entity.

Tasks are placed on a entity.

Command can take arguments. 

command1 {}
command2 { arguments }

### rem
comment

```kdl
rem "this is a comment"
rem "this is also a comment"
```

### flush
Stop other scrips

```kdl
flush
```

### loop
Run specic number of steps, `-1` loops forever.

```kdl
loop 4
{
    command1
    command2
}
```

### wait

wait specified milliseconds

```kdl
wait 3000
```

### if

```kdl
if condition {
    command1
    command2
}
```

### affect
Run a block on another entity. This either flushes or insert it.

```kdl
print "Hello, Fred!"

affect "Fred" "FLUSH"
{
	wait 1000
	print "Leave me alone!"
}
```

### run
Runs a script file on the current instance

### task
Defines a task

```kdl
task "taskname"
{
    command1
    command 2
}
```

### do
Starts a tast
```kdl
do "taskName"
```

### wait
Waits script until task is done
```kdl
wait "taskName"
```
### doWait
Starts and waits until a task is completed
```kdl
dowait "taskName"
```

### sound
Play a sound on a entity channel
```kdl
sound "voice" "player/hello.wav"
```


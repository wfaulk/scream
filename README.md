# scream
## I Have No Mouse, and I Must Scream<sup><a href="#dagger">&dagger;</a></sup>

Do you use text consoles under VMware?  Are you tired of VMware grabbing your
mouse and having to Ctrl-Alt or Ctrl-Cmd out of them in order to work in another
window, despite that the mouse has no function inside the text console?  Do you
need to scream?

### How to use

Run `scream` on the guest once per boot (in a startup script would make sense).
It doesn't make any difference if the program is run within the console or not,
and it doesn't require any elevated guest privileges.  Run `scream -d` to
revert.

### How it works

As you may already know, VMware consoles have two mouse modes: absolute and
relative positioning.  If VMware Tools is not running in the guest, or, more
accurately, no VMware-knowledgeable tool has requested otherwise, the console
runs in relative mode.  This means that the console grabs the mouse and presents
any movement of the mouse directly to the guest, emulating a PS/2 mouse.  In
absolute mode, the console bypasses the PS/2 emulation and sends the absolute
position of the mouse to VMware tools, which then tells the guest where to move
its pointer.

Absolute mode won't work if you don't have VMware Tools installed, because
nothing is listening for that out-of-band mouse data.  So it makes sense that
the console defaults to this mode.  What doesn't make sense is that the console
has to grab the mouse in order to send keyboard data, which, unless I'm
mistaken, is always sent as emulated PS/2 keyboard codes.

I figured that if the guest requested that the console enable absolute mode, it
could then read those out-of-band mouse data and figure out when to give up
mouse control.  As it turns out, it's even simpler than that.  Once you enable
absolute mode, the console deals with figuring out when the mouse pointer has
left the console and automatically ungrabs it.  All the guest has to do,
assuming it doesn't care about the mouse at all, is request absolute mode.  And
that's what, and all, `scream` does.

---

<a name="dagger">&dagger;</a> Inspired by the Hugo Award-winning short story &ldquo;<a
href="http://www.isfdb.org/cgi-bin/title.cgi?41300">I Have No Mouth, and I Must
Scream</a>&rdquo;, by Harlan Ellison

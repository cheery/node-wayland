These are work-in-progress node.js bindings. I'll supply examples and screenshots when it can be used.

Current version builds on: https://github.com/jekstrand/wayland/tree/dispatchers-internal

Task List:

 *   Verify that the Proxy::Marshal doesn't leak memory and works properly in every situation.
 *   Write some examples, see whether the new bindings work properly.
 *   Verify that everything works.
 *   Add animated checkboard output to the simple-shm.c
 *   Find a good method for listener to provide the correct proxy.
 *   Provide interface parser, for users to access weston interfaces from their apps.
 *   Write example for keyboard, include fullscreen/toplevel switching.
 *   Write example for drag&drop and copy&paste.
 *   Find some code that crashes, then put the system go into known state instead of crashing.
 *   Provide wayland-egl bindings and tune cheery's webgl-bindings to work with wayland.
 *   Fill the task list with more tasks when you know what to do.


Bitcoin address for donations: 19Cjd98dNM4SRoNP1t1qbYunYCz8TgjQAm (node-wayland)

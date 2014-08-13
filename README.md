Shake-Listen-Know
=================

Sonification of an android phone’s status. The sonification is presented when the phone is shaken:
* Bluetooth on: whistle* GPS on: horn* Battery level: crowd of clappers, the level is proportional to the number of clappers* Battery charging: the pitch of the clapping changes repeatedly from low to high (“zigzag” sound)

Check the [demo video](http://vimeo.com/30343838)
---

The SLK android app sends specific data to the specified endpoint.
The endpoint runs a Pure Data patch that identifies the shaking movement and processes audio. (Use `ShakeListenKnow.rj/_main.pd`)

---
This was a 3-week project for the Sound in Interaction course at the Royal Institute of Tehcnology (KTH) Stockholm
José Granjo and Helene Oberhumer
Autumn 2011


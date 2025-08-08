### Context

- Minecraft like game with chunk size 16x16x16
- Lighting system should work like in Minecraft. Blocks have transparent value depending on which sunlight is blocked for specific ammount of light levels. For example leaves are transparent, so they reduce light lvl by 1 if sunlight verticaly travels through.
- Light should correctly update and show across chunk borders. Both Horizontaly and verticaly. Transition should be smooth. No block next to each other should have larger light difference then 1. (of course wall in between is exception) Light should transition smoothly just like in minecraft
- For lighting logic check light.c and light.h

### Standarts

- Organize files and code in them according to C language standarts
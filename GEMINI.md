### Context

- Minecraft like game with chunks 16x16x16
- Lighting system should work like in Minecraft. Blocks have opacity depending on which sunlight is blovked for specific ammount of light levels. For example leaves have opacity of 1, so they reduce light lvl by 1 if sunlight verticaly travels through.
- max world height will be y=256. So there should me MAX_WORLD_HEIGHT variable set up and based on that there should be light seeding
- Light should correctly update and show across chunk borders. Both Horizontaly and verticaly. Transition should be smooth. No block next to each other should have larger light difference then 1. (of course wall in between is exception) Light should transition smoothly just like in minecraft
- For lighting logic check light.c and light.h

### Standarts

- Organize files and code in them according to C language standarts
Link to paper: [here](https://drive.google.com/open?id=1vbofrBw5weuCEwZQOMYIOjeT1exQaEEi)

Link to presentation: [here](https://drive.google.com/open?id=0ByrVEUyYZGdLWXhIenpndmQwRTA)

# Thesis
I set out to craft a MMO ARPG from scratch with my own custom C++ engine in 8 months.
All of the code, art, and design was made by me.

Networking System Features:
- Reliable and Unreliable packages
- Ordered and Unordered packages
- Dedicated Server/Listen Server
  - Headless Dedicated Server
- Supports up to ~71 people concurrently

Added debugging overlay for:
- Client-Side Prediction

Game features:
- Networking System
- Player Accounts
- Base Classes (3)
- Leveling System
- UI System
- Persistent World
- Lock-on
- Evil Class (1)
- Evil NPCs
- Boss NPCs
- Crystal Core
- Good NPCs
- Cthulhu Class (1)
- Primary Weapons
- Warping
- Communication
- Partical System

Future Work:
- Entity Component System
- Adaptive Quality of Service
- Multithread Packet Filling
- Delta Compression
- UI Layers

Takeaways
- Approach game creation cocentrically
  - Start with a small core, and keep adding layers
- It takes 3 to 4 times longer to implement networked gameplay features
- Actual playtesting is difficult
- Debugging, Debugging, Debugging
  - Make yourself tools early and often, visualizing problems makes them easier to solve

# How to Play
Start the game
```
...\Thesis\Run_Win32\Thesis.exe
```
Type in a username (your computer name is used by default if not provided), add a password if you want to protect your account.

Press "Dedicated Server" to start a server (you won't be able to play)

Press "Listen Server" to start a server (you will also be playing)

Press "Join Server" to connect to the typed in IP Address (to player with others, you need to be connect via ethernet)

# Controls

[W/S/A/D] moves

[Space] fires

[Click/Drag] interact with the UI 

# Other Resources
[Fundamental Multiplayer RPG Math](https://www.gdcvault.com/play/1015037/Fundamental-Multiplayer-RPG)

[Gaffer on Games](https://gafferongames.com/tags/networking/)

[I Shot You First! Gameplay Networking in Halo: Reach](https://www.gdcvault.com/play/1014345/I-Shot-You-First-Networking)

[Balancing Multiplayer Competitive Games](https://www.gdcvault.com/play/1570/Balancing-Multiplayer-Competitive)

[Maximum Transmission Unit](https://en.wikipedia.org/wiki/Maximum_transmission_unit)

[IPv6 Packet](https://en.wikipedia.org/wiki/IPv6_packet#Fixed_header)

[User Datagram Protocol](https://en.wikipedia.org/wiki/User_Datagram_Protocol)

# K-arnage

K-arnage is a 2D SFML game written as a last-year college project for *Multiplayer-Distributed Programming* module. This is a pair-project.

### Current Status (*master*)
[![Build status](https://ci.appveyor.com/api/projects/status/u91uk4akkcl3377m?svg=true)](https://ci.appveyor.com/project/Alex-Jay/k-arnage-2d-sfml-game)

*Thank you to John Loane for the [Core Framework](https://bitbucket.org/johnloane/gd4201819classcode/src). and Dundalk Institute of Technology for providing insight and resources for this project.*

**Members**:

  - Michael Grinnell
  - Aleksandrs Jevdokimovs

**Degree**: BSc. (Hons) Computer Science in Games Development

# Dependencies

  - x86/x64 build machine
  - SFML 2.5.x or higher
  - Visual Studio 2017 (not necessary)
  - SFML Binaries in Build folder.

**Disclaimer:**
*This game is written for educational-purposes only and is under Open-Source-Licence. You may use this framework to design/implement your own game, remember to give credit to the developers.*

[![Open-Source-Licence](http://www.myiconfinder.com/uploads/iconsets/64-64-04a5e3532f7fb534017d5aeed9941c38.png)](https://opensource.org/licenses)

### Installation & Usage

K-arnage requires SFML 2.5.x or higher.

- Clone the repository and open it in Visual Studio.
- In Visual Studio, **Right-click** on the solution and click **Properties**.
- In **C++** tab, if not set, set up **Additional Include...** path to SFML **include** folder.
- In **Linker** tab, set up the **Additional Librarary Dependencies...** path to SFML **lib** folder.
- Build the project (***x86** recommended for greater platform support*).
- Upon building the project, **copy** files from SFML **bin**folder and **paste** it into **Debug** or **Release** folder in the root path of the project.
- Build & Run the game.
 
Cloning repository (**Git Bash** or **CMD**):

```sh
cd desktop
git clone https://github.com/Alex-Jay/2d_sfml_game.git
```

For Development branch (**Git Bash** or **CMD**):

```sh
cd '2d_*'
git fetch && git checkout Development
```

For Release branch (**Git Bash** or **CMD**):

```sh
cd '2d_*'
git fetch && git checkout Release
```

### Development

Want to contribute? Great!
**E-mail**: alexjaycontact@gmail.com if you would like to be added as a contributor.

### Todos
 - Remove initial game structure.
 - Add two-player on one keyboard.
 - Add Shooting and free-movement.
 - Design level.
 - Stick two-players into one view (*Decide if split-screen is necessary*).
 - Add teleporting functionality (*Power-up*).
 - Record and/or source audio effects and background music.
 - **More to be added.**

### Specification
 - Keyboard and/or Xbox controller input.
 - **States & Menus**. The game should have discrete states such as a Titlestate, Menustate, Gameoverstate. These states should contain interactive menus.
 - **Sprites and animation**. You are not required to build custom resources – feel free to use openly available graphical resources available online and mentioned in the slides.
 - **Collision detection and physics**. When players or objects collide with the edges of the world or solid objects you should deal with this appropriately.
 - **Shaders**. You should improve the look and feel of your game by using shaders.
 - **Music and sound effects**. You should have background music and in game sound effects.
 - **The game should be fun to play.**
 - Give an overview of all of the code with **particular emphasis on the code that you were responsible for writing**. The code should be **clearly commented with a comment at the start of each file to say who was involved in writing it** and a **comment at the start of each function to say who wrote it**. Use version control to track the development of the game and submit the link to your code repository.
 - Discuss how you will make this a networked game.
 
*If you have any further requests or proposals, don't be stumbled to contact us, we love socialising <3.*

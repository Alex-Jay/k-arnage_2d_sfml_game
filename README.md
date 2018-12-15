# K-arnage

K-arnage is a 2D SFML game written as a last-year college project for *Multiplayer-Distributed Programming* module. This is a pair-project.

*Thank you to John Loane and Dundalk Institute of Technology for providing insight and resources for this project.*

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
 
*If you have any further requests or proposals, don't be stumbled to contact us, we love socialising <3.*

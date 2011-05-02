# flQuake
flQuake is a Quake port for the Flash platform. It was based on the Makaqu codebase, a heavily modified Quake engine by Manoel Kasimier. I decided to base flQuake on Makaqu due to the familiarity with the engine when i used to work with Dreamcast homebrew. flQuake has been, so far, an adventure for me, and a pretty exciting one. It uses only the original software renderer and all modifications have been made on top of it.

## Features
* Bilinear texture filtering
* Support for the Half-Life BSP format (BSP version 30)
* Support for Half-Life WAD3 and true-color textures (embedded in BSP only)

## Authors and contributors
* [id Software](http://www.idsoftware.com) (Creators of Quake)
* [Klaus Silveira](http://www.klaussilveira.com) (Creator, developer)
* Michael Rennie (Original Flash port of Quake)
* Manoel Kasimier (Makaqu engine, based on Quake)

## License
[GNU General Public License, version 2](http://www.gnu.org/licenses/gpl-2.0.html)

## Roadmap
* Improve Half-Life BSP support
* Transparent textures

## Status
I'm still working with flQuake, but not as much as before. There are some bugs i need to fix and tons of things to improve. I still need to make the engine run at some decent speeds and clean the codebase, because honestly, it's a mess.

## Todo
* Fix the Windows/Linux build
* Fix weird particle problems
* Fix weird texture problems (specially skies)
* Fix transparency issues
* Clean the codebase from Dreamcast particularities, such as input (this might speed things a little bit)
* Clean the code base from everything that's not needed or doesn't work as expected
* Comment the code: not only the changes, but the entire engine. Always wanted to do this!

## Compiling
You'll need [Adobe Flex](http://www.adobe.com/products/flex/), [Adobe Alchemy](http://labs.adobe.com/technologies/alchemy/) and [Flash Develop](http://www.flashdevelop.org/) to build flQuake. Familiarity with Adobe Alchemy is a minimum requirement. Oh, and it's a real pain in the ass to setup Alchemy on Windows, so, use Linux for this.

## Thanks
* id Software for creating Quake, and most importantly, releasing it's code under the GPL
* Michael Rennie for his Flash port of WinQuake
* Manoel Kasimier for Makaqu

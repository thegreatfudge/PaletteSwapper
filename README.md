# PaletteSwapper

Core problem: find nearest neighbour based on RGB values (number of colors to look through can be quite big).
Solution used: KD Trees - in this case 3D tree used. There were two other possibilites - octotree (which I find was too advanced for this specific problem)
or color reduction to have less colors (by using for example k means).

Sources used: wikipedia page for KD trees, Rosettacode.com <br />

Average times: 16 colors - average 8 seconds <br />
               27 colors - average 9 seconds <br />
               64 colors - average 12 seconds <br />
               128 colors - average 12 seconds <br />
               367524 colors (original obraz-A I received) - average 29 seconds. <br />
               
               

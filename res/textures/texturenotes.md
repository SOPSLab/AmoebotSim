# On Exporting Particle Textures

This is a short guide to exporting textures from `particle.svg` when using [Inkscape](https://inkscape.org/) v0.92+.


## Selecting Textures to Export

1. First, make sure you are on the Texture layer in Inkscape.
2. Next, check and make sure that the Texture layer is unlocked (this can be checked via the lock icon towards the bottom of Inkscape, right next to the dropdown to select layers).
3. Now, using the Select tool (the 1st tool in the list on the left side of Inkscape), select all of the textures that you want to include. This can be done by dragging a box around the textures using the Select tool. You can tell whether or not this has succeeded by a dotted box surrounding
the textures that you want to include.
4. Now, navigate to File -> Export PNG Image (or `Shift+Ctrl+E`, if using the default settings on Inkscape) to get the prompt to export the PNG image.
5. Follow the Dimension Guidelines below when filling out the prompt to export the PNG image.


## Dimension Guidelines

The dimensions of the export format will affect the way the textures on the simulator will render, so to avoid issues with rendering, please follow the below instructions:

1. Set your units to `px`.
2. Make sure that both your `x0` and `y0` fields in the "Export PNG Image" prompt are `0`.
3. Set both the `x1` and `y1` fields to the value `4369.067`.
4. Make sure that both the `Width` and `Height` fields under the Export area (NOT under Image size) are also set to the value of `4369.067`.
5. Under the Image size field, make sure that both the `Width` and `Height` fields are set to `4096` pixels at `90.00` dpi.
6. Make sure you have the correct filename for your PNG file (for `particle.svg`, it should be `particle.png`).
7. Be sure to check the box titled "Hide all except selected" (this will hide the grid and only include the selected textures).
8. Export :)

# Adding Buildings to Stonesense #

This is a brief summary of how to add new building sprites to Stonesense.

Some familiarity with the way [Sprites](Sprites.md) work in general would be useful.

# Sprite graphics #

First new graphics will need to be developed for the buildings. Then they will need to be added to _objects.png_ as per the instructions on [Sprites](Sprites.md). You will need to know the indexes of the sprites. While you can figure these out by counting, there is a shortcut: within Stonesense, hit F10 and the objects.png will be displayed, with index numbers superimposed.

# _buildingname_.xml #

To tell Stonesense what to do with the graphics you have added, you need to define the building in an xml file. Create one with a sensible name in the _buildings_ folder, and add it to the list in _index.txt_

_hint:_ why not make a copy of the most similar building.xml instead of starting from scratch?

# XML format #

Here is an annotated sample of the XML format used in the building.xml

```
<?xml version="1.0" ?> <!-- this lets the xml parser know that your file is xml -->

<Building name="Cabinet" gameID="cabinet">
<!-- Building is the root node of the xml file. You can only have one per file -->
<!-- The name attribute doesnt do much yet, but may be used for UI stuff later -->
<!-- the gameID must match what the debug interface on stonesense returns -->

	<if>
	<!-- The sprites for the building are now chosen using an if/else based
	decision tree.-->

		<MaterialType value="Wood" />
		<!-- The first element of an if or else node is the condition to match-->
		<!-- In this case, it matches on the type of material being wood -->
		<!-- More on these below -->

		<sprite index= 88 />
		<!-- And here's what you were waiting for -->
		<!-- This Sprite node tells Stonesense to use graphic 88 for the
		wooden (remember the 'if' statement?) cabinet -->
		<!-- More tricks to use on this below, too -->

	</if> <!-- The end of this if node -->

	<else> <!-- Another conditional node, 'else' this time -->
	<!-- else nodes dont *have* to have a condition- without one they will 
	match anything left over (you can also keep chaining up elses after an if
	till you match every possibility you need) -->
		<sprite index= 87 />
		<!-- So without a condition tag, this tile will make any cabinet that
		isn't wood use graphic #87 -->
	</else> 
	
</Building> <!-- any xml tag needs a closing tag to match, or to have its own closing slash before the right angle bracket -->

```

# Tile Conditions #

There are two ways to get more complex conditions.

Firstly, anywhere you can put a **sprite**, you could put an **if**/**else** group (or several).

Secondly, some of the condition options can contain children within them, allowing boolean logic
```
<and>
	<PositionIndex value=1 />
	<MaterialType value="Wood" />
</and>
```

Note that an **else** needs to be hooked up to an **if**, but an **if** can stand alone.

## List of Condition Options ##

  * **PositionIndex**: Matches on the location of a tile within a building. Used in multitile buildings such as workshops. Less effective on anything with non-fixed dimensions. The index given by the value attribute is from west to east then north to south, and is zero based, eg:
> | 0 | 1 | 2 |
|:--|:--|:--|
> | 3 | 4 | 5 |
> | 6 | 7 | 8 |

```
<PositionIndex value=1 />
```

  * **MaterialType**: Name of a type of material that the construction is made from. This is general material only, not specific subtype. Sometimes the material may be surprising, such as all forges are considered 'metal' because it takes the material type from the anvil.
> | Wood | Stone | Metal | Leather | Silk |
|:-----|:------|:------|:--------|:-----|
> | PlantCloth | GreenGlass | ClearGlass | CrystalGlass |

```
<MaterialType value="stone" />
```

  * **NeighbourWall**: Detects whether one of the cells neighbors is solid rock or constructed wall. Good for determining orientations of doors and similar. Remember that you can use boolean logic to eg search for a wall the the north **and** one to the west.
> | None | North | South | West | East |
|:-----|:------|:------|:-----|:-----|

```
<NeighbourWall dir="West" />
```

  * **NeighbourSameType**: Detects whether one of the cells neighbors is of the same type of building. Note that it does **not** pay any attention to whether is is actually part of the same building, or just another built alongside. Useful for edges of buildings that should merge with their neighbors, or where PositionIndex wont work due to changing shape. Use boolean logic for more complex neighbor arrangements.
> | None | North | South | West | East |
|:-----|:------|:------|:-----|:-----|

```
<NeighbourSameType dir="West" />
```

  * **NeighbourSameBuilding**: Detects whether one of the cells neighbors is another part of the same (multitile) building. Mostly useful when trying to figure out the orientation of the building. Use boolean logic for more complex neighbor arrangements.
> | None | North | South | West | East |
|:-----|:------|:------|:-----|:-----|

```
<NeighbourSameBuilding dir="West" />
```

  * **NeighbourOfType**: Detects whether one of the cells neighbors is of a given type of building. Use boolean logic for more complex neighbor arrangements.
> | None | North | South | West | East |
|:-----|:------|:------|:-----|:-----|

```
<NeighbourOfType dir="West" value="gear_assembly" />
```

  * **BuildingOccupancy**: Detects the building occupancy value of the building. The building occupancy value can be useful in determining orientation and state of buildings. eg 3 means the solid part of a workshop, 6 means the raised part of a bridge, 2 means under construction...

```
<BuildingOccupancy value=1 />
```

  * **NeighbourIdentical**: Ensures a neighbor is part of the same building, and has the same Occupancy flags.
> | None | North | South | West | East |
|:-----|:------|:------|:-----|:-----|

```
<NeighbourIdentical dir="West" />
```

  * **HaveFloor**: Some rare buildings may not have a floor underneath. This will detect that condition

```
<HaveFloor/>
```

  * **FluidBelow**: Returns true if the tile **below** this one contains the value or more of fluid.

```
<FluidBelow value=4 />
```

  * **always**: Pretty straightforward. May seem redundant, but can occasionally be useful, especially when leading up to included files.

```
<always />
```

  * **never**: Pretty straightforward. May seem redundant, but can occasionally be useful, especially when leading up to included files.

```
<never />
```

  * **and**: Returns true if all of its children are true.

```
<and>
  ...More conditions...
</and>
```

  * **or**: Returns true if any of its children are true.

```
<or>
  ...More conditions...
</or>
```

  * **not**: Returns the opposite of its child.

```
<not>a condition</not>
```

  * **MaterialIndexCondition**: Currently based on index, this one needs more work.

  * **AnimationFrame**: Used for A4, but being phased out now.

# Advanced Sprites #

Firstly, a tile can contain multiple sprites. This is useful where parts of a tile can be used repeatedly. The bench in a workshop, for example, is separate from the tools and objects. By drawing a bench, then drawing eg a tool, the same bench can be used repeatedly, and can be swapped out for benches of other materials.

A tile can also contain no sprites, which is how you get an undrawn tile instead of a default box.

To assist in combining sprites, it is possible to offset the sprites before they are drawn. Adding an offsetx and offsety attribute with appropriate integer values to the Sprite tag will achieve this.

```
<sprite index=43 offsetx=4 offsety=5 />
```

_hacky trick:_ Sprites with offsets are clipped **before** the offset is applied, of course. This means that they can be drawn outside the boundaries of the original sprite. So if you **really** needed a tile to be drawn taller than 32 pixels, you could draw the lower part with one sprite, and the upper part with another, offset, sprite. Of course, it wouldn't **really** be taller, and thus might interact with the rest of the terrain in strange ways.

# Sprite image files #

Any of the building, if/else or sprite tags may have a file attribute, that loads an alternative image file to the default.

```
<building name="Magma Forge" gameID="magma_forge" file="workshop.png">
```

This applies to all sprites within that element (unless a child overrides it with its own file declaration).

# Include files #

Anywhere you could have an if/else node or sprite, you can have an include element

```
<include file="wsUpperBorder.xml"/>
```

This will swap in the contents of the include file for that element.

Note that for predictability, the include file has its own image file hierarchy, so will not change the sprites drawn regardless of what file choices the includer makes.
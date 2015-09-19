# Adding Creatures to Stonesense #
Adding new creatures to Stonesense is fairly easy, but before reading this guide I suggest you familiarize yourself with how Stonesense deals with creature [Sprites](Sprites.md).

## Old Creature Config ##

## Creatures.xml ##
Stonesense is fully configurable in the way it renders creatures. No information is hardcoded, but rather loaded from the creature database found in **Creatures.xml**.

The structure of this file is fairly simple:
```
<Creature gameID="dwarf"        sheetIndex = 1  /> 
<Creature gameID="cat"          sheetIndex = 20 /> 
<Creature gameID="dog"          sheetIndex = 21 /> 
```

As you can see, each creature is identified by a **'gameID'** attribute. This has to match one of the known creature types in Dwarf Fortress, see [List of Creatures](AddingCreatures#List_of_Creatures.md). This gameID tag is not case sensitive. If Stonesense is unable to match your gameID with one from Dwarf Fortress, a log message will be written to _Stonesense.log_ upon loading a map.

When trying to render a creature, Stonesense will look up its gameID in this table and render the sprite from the creatures sprite sheet with the matching **Sheet Index**.

Now there is nothing wrong with reusing a sprite. Consider this:
```
<Creature gameID="GIBBON_WHITE_HANDED"  sheetIndex = 42  /> 
<Creature gameID="GIBBON_BLACK_HANDED"  sheetIndex = 42 /> 
```

### Example: ###
We are going to add a new creature. A one-humped camel. The first we have to do is figure out it's gameID. Which happens to be **'camel\_1\_hump'**. Then we pick out an unoccupied sheet index to put your sprite in. Let's say 18.

This means we have to draw our camel at this location (the grid was added for readability):
![http://stonesense.googlecode.com/svn/wiki/img/creature_newcre_pos.png](http://stonesense.googlecode.com/svn/wiki/img/creature_newcre_pos.png)

Now all we have to do is add the following line to Creatures.xml:
```
<Creature gameID="camel_1_hump"      sheetIndex = 18 /> 
```

## Professions ##

Creatures elements may have a set of child Profession nodes, which define different sprites for professions and/or genders of a creature type.

```
<Creature gameID="DWARF" 		sheetIndex = 180 >
	<Profession name="Miner" sheetIndex = 181/>
	<Profession name="Woodworker" sex="M" sheetIndex = 182/>
	<Profession sex="F" sheetIndex = 140/>
</Creature>
```


---


## New Creature Config ##

Relevent to the next version, rather than A4.

Stonesense is fully configurable in the way it renders creatures. No information is hardcoded, but rather loaded from the creature databases found in the creatures directory.

## index.txt ##

The first thing the game does is look in the index.txt file. It uses this file to tell it what configuration files are available, and in what order to load them (this becomes important later)

## Creatures.xml ##

The index file identifies a number of xml files

The structure of this file is ~~fairly simple~~ rapidly becoming rather convoluted as we add features:
```
<?xml version="1.0" ?> <!-- this lets the xml parser know that your file is xml -->
<creatures> 
<!-- this helps to identify the xml file, and allows it to contain
multiple creature definitions while remaining valid xml -->

	<creature gameID="CAT" 		sheetIndex = 20 /> 
	<creature gameID="DOG" 		sheetIndex = 21 />
	<!-- Simple creature definitions just match a creature type name (as in the
	raws) to its sprite index -->

	<creature gameID="DWARF" sheetIndex = 180 file="creatures/dwarves.png" >
		<variant name="Miner" sex="M" sheetIndex = 181/>
	</creature>
	<!-- More complex creatures use a number of variant elements to describe
	multiple sprites available for different species members -->

	<creature gameID="BAT_GIANT" 		sheetIndex = 40 > 
		<variant sheetIndex=41 frames="12"/>
		<variant sheetIndex=42 frames="45"/>
	</creature>
	<!-- another complex example, this one uses variants to display different
	frames of animation -->

</creatures>

```

As you can see, each creature is identified by a **'gameID'** attribute. This has to match one of the known creature types in Dwarf Fortress, as defined in the `[CREATURE:critter_name]` part of the raws. This gameID tag is case sensitive. If Stonesense is unable to match your gameID with one from Dwarf Fortress, a log message will be written to _Stonesense.log_ upon loading a map.

When trying to render a creature, Stonesense will look up its gameID in this table and render the sprite from the creatures sprite sheet with the matching **Sheet Index**.

Now there is nothing wrong with reusing a sprite. Consider this:
```
<creature gameID="GIBBON_WHITE_HANDED" 	sheetIndex = 42  /> 
<creature gameID="GIBBON_BLACK_HANDED" 	sheetIndex = 42 /> 
```

### Example: ###
We are going to add a new creature. A one-humped camel. The first we have to do is figure out it's gameID. Which happens to be **'CAMEL\_1\_HUMP'**. Then we pick out an unoccupied sheet index to put your sprite in. Let's say 18.

This means we have to draw our camel at this location (the grid was added for readability):

![http://stonesense.googlecode.com/svn/wiki/img/creature_newcre_pos.png](http://stonesense.googlecode.com/svn/wiki/img/creature_newcre_pos.png)

Now all we have to do is add the following line to Creatures.xml (inside the creatures tag):
```
<creature gameID="CAMEL_1_HUMP"      sheetIndex = 18 /> 
```

## More Advanced Sprites ##
### Variants ###

The variant element encompasses anything more specific than the creature type. Currently supported:

  * **profession** This checks the creatures profession, based off the strings in Memory.xml

  * **custom** This checks against custom professions you give your dwarves rather than the built in types. Use sparingly, as string matches can be rather slow.

  * **special** checks some unusual conditions. Currently checks for "Normal", "Zombie" or "Skeleton"

  * **sex** checks against "M" for male or "F" for female.

  * **frames** defines animations by setting variants that are displayed at different times. The attribute should contain a list of the frames 0-5 that the sprite should be used. eg frames="012" for the first half of the animation sequence.

### Multiple Definition Files ###

Stonesense searches for images in the order that they are found top to bottom in the index file, then in the definition files. So if you are adding a custom set of sprites for something already in the system, you need to add it to the **top** of the index file.

### Multiple Image Files ###

You can make use of image files other than the default by adding a file attribute to either the creature or creatures element. (Putting it on the creatures element will change the default for the tile, but each creature element can still select its own).

### Tricks ###

While simple creature definitions arent much use without a sheetIndex attribute, it can be useful to define variants without giving the base creature a sprite. This would cause the game to keep looking for creature definitions, allowing you to, for example, define custom professions in one file while still using the basic definitions.
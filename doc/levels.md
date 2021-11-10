# Levels

Levels are edited in Blender v2.93. There are 6 major concepts you should understand

* Materials
* World Geometry
* World Boundary
* Bases
* Decor
* Pathfinding

## Materials

The level importer ignores all material properties except the name. Materials are defined by name in a separate file outside the level editor. To give an object a specific material the name must match exactly. At the time or writing this the following materials are defined

* Ground
* Rock
* Metal
* Crate

## World Geometry

Any Object whose name starts with `Geometry` will be rendered as an intangible object in the scene. Any such object should be located at the origin with no rotation or scaling to the object. Any desired transformation should be applied directly to the mesh and not the object.

## World Boundary

There should be exactly one object named `Boundary` in the level. It should consist of a mesh that contains only vertical segments or walls that surround the level. The boundary should be a convex shape when viewed from above.

## Bases

Any object that starts with the name `Base # ` where `#` is a team number (starting at 0) will be replaced with a base for the cooresponding team. A team number of `4` will be interpreted as a neutral base.

## Decor

Any object whose mesh name starts with `Decor` followed by a space will be interpreted as a decoration object. If a decoration object has a cooresponding mesh named `DecorBoundary` then the decor will use that mesh to define the collision for the decor object. Like the world boundary, the `DecorBoundary` mesh should consist entirely of vertical segments that when viewed from above form a convex shape.

## Pathfinding

That should be at most 1 object with the name `Pathfinding`. It should be a mesh that defines path for the AI to travel. Try to keep the number of vertices low probably less than 32, as too many vertices could effect performance. The mesh that define paths should consist of a bunch of point connected together. Lines connecting points should be clear of obstacles. All points should be reachable for any other point. The AI will use the connections between points to determine where it can travel to avoid obstacles. The mesh should also be extruded upwards so each point is actually 2. One point on the ground and one at least 1 unit above the other point. The lines connecting points will then become faces instead of just lines. This weird quirk is due to a limitation on the process of exporting to fbx then importing with assimp. Somewhere along the way lines are removed so to preseve connections they need to be polygons.

## LevelTemplate.blend

Use [LevelTemplate.blend](./LevelTemplate.blend) as a starting point. You can edit the geometry defined in that file as well as duplicate bases, and decor and place them anywhere on the map.
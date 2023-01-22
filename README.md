# AngryProceduralTools

This Repository requires the [Angry Utility](https://github.com/AngryLizard/AngryUtility).
Contains a bunch of mesh generation tools, look at the `Content/Examples` content folder.

Generally, create a `ProceduralActor` actor class and override `GenerateMesh`. This function will act like a custom mesh generation graph: You pass a mesh object through multiple filters from the `Procedural Mesh` category, then pass it as return statement where it gets applied to a procedural mesh. The `ProceduralActor` actor also automatically gets in-editor controls to simplify baking static meshes from the procedural mesh.

Alternatively this can be done in other Blueprint classes using the `Apply To Meshes` function, but static mesh ocnversion is not available there.

Procedural Gradient textures are currently broken due to a faulty engine upgrade.
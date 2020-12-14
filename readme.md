# GeneralShapeFormation guideline

We distinguish different phases (which can depend on each other). For example:

- chain movement primitive
- triangle expand primitive
- ...

Each phase is defined in its own file, e.g. `selfmade/generalshapeformation/chain_phase.cpp`. 

Every method, struct, etc. belonging to a specific phase is prefixed with the name of that phase. E.g. `chain_DepthToken`

Each phase has at least its own activate function, e.g. `chain_activate()`

Keep your code DRY :)

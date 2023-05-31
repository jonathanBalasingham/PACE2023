# PACE2023
PACE 2023

## Description of solver
The solver consists of the following steps:
1. Modular Decomposition is computed for the graph
2. For each module, a random node is selected to start at; From this node, we search the neighbors for the nodes with the smallest symmetric difference.
3. The `n` nodes with the smallest symmetric difference are collapsed with the current node.
4. Back to step 2 if there are any neighbors left. If there are still nodes in the graph but no neighbors, it is disconnected, another node is selected and step 2 is continued. 

## Usage
To build the project simply run:
`cmake --build ./build --target PACE`
from the root directory. The compiled binary should be placed in the `build/` directory.

The usage for the output binary:
`PACE <graph>.gr`
The resulting `tww` file will be output to the same directory from which the binary is run. The name of the output file will match the input graph file with the extension changed from `.gr` to `.tww`.

## Release
A compiled binary is included in the release. You may need to run `chmod u+x PACE` before being able to run it.

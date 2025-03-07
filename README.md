# Abalone State Space Generator

This program generates all possible legal next-ply moves and the resulting game board configurations for the Abalone game.

## Patch Notes
### Patch 1.0.1
- Pushed March 7, 2025 9:06pm ; Wayne Chen
- Merged Logan's repo with Project repo
- Added patch notes back
- Updated gitignore
### Patch 1.0.0
- Pushed March 6, 2025 2:25pm ; Sam Holtzman
- Initial Project Setup
    -  Added file / directory structure

## Building the Program

To build the program, run the following command:

```bash
make
```

This will create an executable called `abalone_generator`.

## Running the Program

To run the program, use the following command:

```bash
./abalone_generator <input_file>
```

For example:

```bash
./abalone_generator Test1.input
```

This will generate two output files:
- `Test1.move`: Contains all possible legal next-ply moves
- `Test1.board`: Contains all resulting game board configurations

## Input Format

The input file should contain two lines:
- First line: A single character ('b' or 'w') denoting the color of the marbles to be moved
- Second line: A comma-separated list of marble positions in the format `<Row><Col><color>`

Example:
```
b
C5b,D5b,E4b,E5b,E6b,F5b,F6b,F7b,F8b,G6b,H6b,C3w,C4w,D3w,D4w,D6w,E7w,F4w,G5w,G7w,G8w,G9w,H7w,H8w,H9w
```

## Output Format

### Move File (Test<#>.move)

Each line represents a move in the format:
- For single marble moves: `<from_position>-<to_position>`
- For group moves: `<from_position1><from_position2>...-<to_position1><to_position2>...`

### Board File (Test<#>.board)

Each line represents a resulting board configuration in the same format as the input, with marbles ordered by color (black before white), then by rows, and then by columns.

## Testing

To run the program with the provided test files, use:

```bash
make test
```

This will generate the output files for Test1.input and Test2.input.

## Design Notes

The implementation uses the following components:

1. Position struct: Represents a position on the hexagonal board
2. Marble struct: Represents a marble with a position and color
3. Move struct: Represents a move with a list of "from" and "to" positions
4. Board class: Represents the game board with methods for generating moves and applying them

The move generation is divided into three types:
1. Single marble moves
2. In-line moves (straight line with 2 or 3 marbles)
3. Side-step moves (moving a group of aligned marbles sideways)

The program ensures all moves are legal according to the rules of Abalone.

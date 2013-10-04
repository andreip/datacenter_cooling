/*
 * -----
 * RULES
 * -----
 *
 * We have some rooms in our datacenter, and we need to connect them all
 * with a single cooling duct.
 *
 * Here are the rules:
 * The datacenter is represented by a 2D grid.
 * Rooms we own are represented by a 0.
 * Rooms we do not own are represented by a 1.
 * The duct has to start at the air intake valve, which is represented
 * by a 2.
 * The duct has to end at the air conditioner, which is represented by a
 * 3.
 * The duct cannot go in multiple directions out of the intake or the AC
 * - they must be the two endpoints of the duct.
 *   The duct must pass through each room exactly once.
 *   The duct cannot pass through rooms we do not own.
 *   The duct can connect between rooms horizontally or vertically but
 *   not diagonally.
 *
 *   Here is an example datacenter:
 *
 * 2  0  0  0
 *
 * 0  0  0  0
 *
 * 0  0  3  1
 *
 * 2--0--0--0
 *          |
 * 0--0--0--0
 * |
 * 0--0--3  1
 *
 * or
 *
 * 2  0--0--0
 * |  |     |
 * 0  0  0--0
 * |  |  |
 * 0--0  3  1
 *
 * --------
 * ABSTRACT
 * --------
 *
 * Will do a DFS exploration that backtracks to find all possible
 * paths of exploration. But in order to speedup the DFS exploration,
 * will remove some bad paths using some strategies/heuristics.
 *
 * --------------------
 * WORLD REPRESENTATION
 * --------------------
 *
 * Will reprosent the world from 2D as a vector. E.g. (this will not
 * actaully get represented, will store 0's and 1's only):
 *
 * 2  0  0  0
 *
 * 0  0  0  0
 *
 * 0  0  3  1
 *
 * will become [2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3, 1].
 *
 * We will be using a plain DFS exploration, but with additional
 * heuristics (two) to be able to fail fast for dfs paths that would
 * fail later on. So we're trying to determine the DFS path fails as
 * early as possible.
 *
 * ----------
 * STRATEGIES
 * ----------
 *
 * 1) You want the exploration path of the DFS to not have corners,
 * which means to have a vertex with a degree of one. The degree can be
 * one of UP, DOWN, LEFT, RIGHT.
 *
 * E.g.
 *
 *   (*)
 * 2  0  0  0
 * |
 * 1--1--1  0
 *
 * 0  0  3  1
 *
 * is already a wrong path because the 0 market with a (*) above has a degree
 * of one (can go only to RIGHT from that room). This means that when
 * we'll get to that room we won't be able to get out of it, as we've
 * entered into it from its right already (we can't walk through rooms
 * we've already visited).
 *
 * 2) You can't create cycles in the path, or make some rooms
 * unreachable.
 *
 *           UP
 *           |
 *  LEFT -- Room -- RIGHT
 *           |
 *          DOWN
 *
 * Because we'll move in this order of priority from one room to
 * another: UP, RIGHT, DOWN, LEFT, then in order to be sure that we
 * don't block some rooms with our DFS path, we need to go as DOWN as we
 * can and check if we can _reach_ all the rooms with a DFS exploration
 * from that end. E.g.
 *
 * 2--0  0  0
 *    |
 * 0  0  0  0
 *    |
 * 0  0  3  1
 *
 * we would be checking from 3 (the most DOWN element which is OUR
 * room) that we can reach the 0's that are not connected through a
 * path. And we cannot reach to all of them:
 *
 * 2--0  0--0
 *    |  |  |
 * 0  0  0  0
 *    |  |
 * 0  0  3  1
 *
 * This means the DFS path we're onto right now is bad, and we need to
 * stop it.
 *
 * Using these two tactics, we'll be stoping when the length of the PATH
 * found equals the total number of unocupied rooms that are OURS, and
 * when the last room is the END room, by starting from the START room.
 *
 */
#include <stdio.h>
#include <stdlib.h>

int rows, cols, start, end;
/* Used to keep track of the visited path of the DFS we'll backtrack.
 * Also the unusable rooms will be marked as visited from the start.
 */
char *visited;
/* The rooms we can reach from the DOWNmost part, so we don't form bad
 * paths. Check the STRATEGIES section.
 */
char *reachable;

int main() {
  scanf("%d %d\n", &cols, &rows);
  int i, crt;

  /* Memory for these three arrays holding the world state. */
  visited = calloc(sizeof(char), rows*cols);
  reachable = calloc(sizeof(char), rows*cols);

  for (i = 0; i < cols * rows; ++i) {
    scanf("%d", &crt);
    switch (crt) {
      case 1:
        visited[i] = crt;
        break;
      case 2:
        start = i;
        break;
      case 3:
        end = i;
        break;
    }
  }

  free(visited);
  free(reachable);

  return 0;
}

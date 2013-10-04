/*
 * AUTHOR: @andreip Andrei-Niculae Petre
 *
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
 * I won't be implementing the 2nd strategy though, it's got an
 * acceptable speed for the big test.
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
 * (IMPLEMENTED)
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
 * (NOT IMPLEMENTED)
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

int width, height, start, end, path_length;
/* Used to keep track of the visited path of the DFS we'll backtrack.
 * Also the unusable rooms will be marked as visited from the start.
 */
char *visited;

/* Test if two rooms are on the same row. */
#define SAME_ROW(a, b)    (a / width == b / width)

#define UP(pos)           (pos - width)
#define DOWN(pos)         (pos + width)
#define RIGHT(pos)        (pos + 1)
#define LEFT(pos)         (pos - 1)

/* Can go UP/DOWN/LEFT/RIGHT in the bounds of the datacenter. */
#define CAN_GO_UP(pos)    (UP(pos) >= 0)
#define CAN_GO_DOWN(pos)  (DOWN(pos) < width * height)
#define CAN_GO_RIGHT(pos) (RIGHT(pos) < width * height &&\
                           SAME_ROW(pos, RIGHT(pos)))
#define CAN_GO_LEFT(pos)  (LEFT(pos) >= 0 && SAME_ROW(pos, LEFT(pos)))

#define IS_FREE(pos, vector)  (vector[pos] == 0)

/* Signature to be able to call function from anywhere in here. */
int count_paths(int, int);

int go_into_room(new_room, length) {
  /* Mark the room as visited. */
  visited[new_room] = 1;
  int number_of_paths = count_paths(new_room, length);
  /* When we return from recursivity we want to cleanup the
   * visited room, as we may visit it again through a different path.
   */
  visited[new_room] = 0;
  return number_of_paths;
}

/* The first of the STRATEGIES section from the above
 * documentation. Read details there.
 */
int path_has_rooms_with_degree_lt_2(int crt) {
  int degree, i;

  for (i = 0; i < width * height; ++i) {
    degree = 0;
    /* Check that we don't have any rooms with degree one
     * which we're not next to from the current room we're into.
     * The hamiltonian path would be impossible to finish because
     * we would not be able to go into the room (with degree one) and
     * also exit from it into a different room (as all its neighbours have
     * been visited, since it's got a degree of one).
     */
    if (IS_FREE(i, visited) && i != end) {
      /* If the _i_ room is neighbour of our current room, then we
       * might be able to visit it right now, so add a degree to it.
       */
      if (CAN_GO_UP(i) && (IS_FREE(UP(i), visited) || UP(i) == crt))
        ++degree;
      if (CAN_GO_RIGHT(i) && (IS_FREE(RIGHT(i), visited) || RIGHT(i) == crt))
        ++degree;
      if (CAN_GO_DOWN(i) && (IS_FREE(DOWN(i), visited) || DOWN(i) == crt))
        ++degree;
      if (CAN_GO_LEFT(i) && (IS_FREE(LEFT(i), visited) || LEFT(i) == crt))
        ++degree;

      /* The DFS path is wrong, so stop from exploring it any further. */
      if (degree < 2)
        return 1;
    }
  }

  return 0;
}

/* @args:
 *   crt: the current position where the DFS path has arrived. When
 *        this reaches the end position, we're done.
 *   length: the length of the DFS path has to equal the total number
 *           of rooms we need to go through.
 *
 * @return:
 *   the number of hamiltonian paths (through all our rooms) from start to
 *   end.
 */
int count_paths(int crt, int length) {
  /* We've found a path. */
  if (crt == end && length == path_length)
    return 1;

  /* The DFS path is wrong, stop it early. */
  if (path_has_rooms_with_degree_lt_2(crt))
    return 0;

  int number_of_paths = 0;
  if (CAN_GO_UP(crt) && IS_FREE(UP(crt), visited))
    number_of_paths += go_into_room(UP(crt), length + 1);

  if (CAN_GO_RIGHT(crt) && IS_FREE(RIGHT(crt), visited))
    number_of_paths += go_into_room(RIGHT(crt), length + 1);

  if (CAN_GO_DOWN(crt) && IS_FREE(DOWN(crt), visited))
    number_of_paths += go_into_room(DOWN(crt), length + 1);

  if (CAN_GO_LEFT(crt) && IS_FREE(LEFT(crt), visited))
    number_of_paths += go_into_room(LEFT(crt), length + 1);

  return number_of_paths;
}

int main() {
  scanf("%d %d\n", &width, &height);
  int i, crt;

  /* Memory for these three arrays holding the world state. */
  visited = calloc(sizeof(char), height * width);

  for (i = 0; i < width * height; ++i) {
    scanf("%d", &crt);
    switch (crt) {
      case 1:
        visited[i] = crt;
        break;
      case 2:
        start = i;
        /* We're at the start position from the beginning. */
        visited[start] = 1;
        break;
      case 3:
        end = i;
        break;
    }

    /* The number of rooms we need to cover with the paths. */
    if (crt != 1)
      ++path_length;
  }

  /* Solve for the number of paths. */
  printf("%d\n", count_paths(start, 1));

  free(visited);

  return 0;
}

layout left:10x12+0+0 \
       right:10x12+10+0 \
       overlap:10x12+5+6
=comment
            1111111111
  01234567890123456789
 0+left----++right---+
 1|        ||        |
 2|        ||        |
 3|        ||        |
 4|        ||        |
 5|        ||        |
 6|    +overlap-+    |
 7|    |   ||   |    |
 8|    |   ||   |    |
 9|    |   ||   |    |
10|    |   ||   |    |
11+----|---++---|----+
12     |        |
13     |        |
14     |        |
15     |        |
16     |        |
17     +--------+
=end

check_horizontal_layout 20x12+0+0 \
			10x6+5+12
=comment
            1111111111
  01234567890123456789
 0+------------------+
 1|                  |
 2|                  |
 3|                  |
 4|                  |
 5|                  |
 6|                  |
 7|                  |
 8|                  |
 9|                  |
10|                  |
11+------------------+
12     +--------+
13     |        |
14     |        |
15     |        |
16     |        |
17     +--------+
=end

check_vertical_layout 5x12+0+0 \
		      10x18+5+0 \
		      5x12+15+0
=comment
            1111111111
  01234567890123456789
 0+---++--------++---+
 1|   ||        ||   |
 2|   ||        ||   |
 3|   ||        ||   |
 4|   ||        ||   |
 5|   ||        ||   |
 6|   ||        ||   |
 7|   ||        ||   |
 8|   ||        ||   |
 9|   ||        ||   |
10|   ||        ||   |
11+---+|        |+---+
12     |        |
13     |        |
14     |        |
15     |        |
16     |        |
17     +--------+
=end

########################################################################

window 6x7+2+3
=comment
            1111111111
  01234567890123456789
 0+left----++right---+
 1|        ||        |
 2|        ||        |
 3| +win.+ ||        |
 4| :    : ||        |
 5| :    : ||        |
 6| :  +overlap-+    |
 7| :  | : ||   |    |
 8| :  | : ||   |    |
 9| +..|.+ ||   |    |
10|    |   ||   |    |
11+----|---++---|----+
12     |        |
13     |        |
14     |        |
15     |        |
16     |        |
17     +--------+
=end

RLayoutFindTopBottomEdges 0 11
RLayoutFindLeftRightEdges 0 19

RLayoutFindMonitorTopEdge    0
RLayoutFindMonitorBottomEdge 11
RLayoutFindMonitorLeftEdge   0
RLayoutFindMonitorRightEdge  9

RLayoutFull       20x12+0+0
RLayoutFullHoriz  20x7+0+3
RLayoutFullVert   6x12+2+0
# greater window area is in "left" monitor
RLayoutFull1      10x12+0+0
RLayoutFullHoriz1 10x7+0+3
RLayoutFullVert1  6x12+2+0

########################################################################

window 6x7+7+3
=comment
            1111111111
  01234567890123456789
 0+left----++right---+
 1|        ||        |
 2|        ||        |
 3|      +win.+      |
 4|      : || :      |
 5|      : || :      |
 6|    +overlap-+    |
 7|    | : || : |    |
 8|    | : || : |    |
 9|    | +.||.+ |    |
10|    |   ||   |    |
11+----|---++---|----+
12     |        |
13     |        |
14     |        |
15     |        |
16     |        |
17     +--------+
=end

RLayoutFindTopBottomEdges 0 17
RLayoutFindLeftRightEdges 0 19

RLayoutFindMonitorTopEdge    0
RLayoutFindMonitorBottomEdge 11
RLayoutFindMonitorLeftEdge   5
RLayoutFindMonitorRightEdge  14

RLayoutFull       20x12+0+0
RLayoutFullHoriz  20x7+0+3
RLayoutFullVert   6x18+7+0
# greater window area is in "overlap" monitor
RLayoutFull1      10x12+5+6
RLayoutFullHoriz1 10x4+5+6
RLayoutFullVert1  6x12+7+6

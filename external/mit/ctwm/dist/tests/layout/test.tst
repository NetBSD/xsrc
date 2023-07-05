layout 0:30x20+0+5 \
       1:30x20+30+5 \
       3:15x25+60+0

=comment
                                                            +3------------+
                                                            |             |
                                                            |             |
                                                            |             |
                                                            |             |
+0---------------------------++1---------------------------+|             |
|                            ||                            ||             |
|                            ||                            ||             |
|                            ||                            ||             |
|                            ||                            ||             |
|                            ||                            ||             |
|                            ||                            ||             |
|                            ||                            ||             |
|                            ||                            ||             |
|                            ||                            ||             |
|                            ||                            ||             |
|                            ||                            ||             |
|                            ||                            ||             |
|                            ||                            ||             |
|                            ||                            ||             |
|                            ||                            ||             |
|                            ||                            ||             |
|                            ||                            ||             |
|                            ||                            ||             |
+----------------------------++----------------------------++-------------+
=end

check_horizontal_layout 75x20+0+5 15x5+60+0
check_vertical_layout  15x25+60+0 60x20+0+5

#
# window in monitor 0
window 20x10+5+10

RLayoutFindTopBottomEdges 5 24
RLayoutFindLeftRightEdges 0 74

RLayoutFindMonitorBottomEdge 24
RLayoutFindMonitorTopEdge    5
RLayoutFindMonitorLeftEdge   0
RLayoutFindMonitorRightEdge  29

RLayoutFull       75x20+0+5
RLayoutFullHoriz  75x10+0+10
RLayoutFullVert   20x20+5+5
RLayoutFull1      30x20+0+5
RLayoutFullHoriz1 30x10+0+10
RLayoutFullVert1  20x20+5+5

#
# window in monitor 1
window 20x10+35+10

RLayoutFindTopBottomEdges 5 24
RLayoutFindLeftRightEdges 0 74

RLayoutFindMonitorBottomEdge 24
RLayoutFindMonitorTopEdge    5
RLayoutFindMonitorLeftEdge   30
RLayoutFindMonitorRightEdge  59

RLayoutFull       75x20+0+5
RLayoutFullHoriz  75x10+0+10
RLayoutFullVert   20x20+35+5
RLayoutFull1      30x20+30+5
RLayoutFullHoriz1 30x10+30+10
RLayoutFullVert1  20x20+35+5

#include "ctwm.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

#include "r_layout.h"
#include "r_area_list.h"
#include "r_area.h"

static char *
trim_prefix(char *str, char *prefix)
{
	int prefix_len = strlen(prefix);

	if(strncmp(str, prefix, prefix_len) == 0) {
		str += prefix_len;
	}

	return str;
}

static char *
trim_spaces(char *str)
{
	char *end;

	while(*str == ' ' || *str == '\t') {
		str++;
	}

	end = str + strlen(str) - 1;

	while(end >= str && (*end == '\n' || *end == ' ' || *end == '\t')) {
		end--;
	}
	end[1] = '\0';

	// translate \t to space
	for(end = str; *end; end++)
		if(*end == '\t') {
			*end = ' ';
		}

	return str;
}

static void
print_rarea_list(RAreaList *list)
{
	printf("[len=%d", list->len);

	for(int i = 0 ; i < list->len ; i++) {
		RArea area = list->areas[i];
		printf(" %dx%d+%d+%d", area.width, area.height, area.x, area.y);
	}

	printf("]");
}

static int
cmp_rarea_list(char *filename, int linenum, char *function,
               RAreaList *got, RAreaList *expected)
{
	if(got->len != expected->len
	                || memcmp(got->areas, expected->areas, got->len * sizeof(got->areas[0]))) {
		printf("%s:%d: %s failed\n"
		       "\t     got: ", filename, linenum, function);
		print_rarea_list(got);
		printf("\n"
		       "\texpected: ");
		print_rarea_list(expected);
		printf("\n");
	}

	return 1;
}

static int
extract_geometry(char *buf, char ***names, int *num_names,
                 unsigned int *x, unsigned int *y,
                 unsigned int *width, unsigned int *height)
{
	char name[32];

	// {monitor_name}:{width}x{height}+{x}+{y}
	if(sscanf(buf, "%31[^:]:%ux%u+%u+%u", name, width, height, x, y) == 5) {
		if(names == NULL) {
			*names = malloc(2 * sizeof(char *));
		}
		else {
			(*num_names)++;
			*names = realloc(*names, (*num_names + 1) * sizeof(char *));
		}

		if(*names == NULL) {
			perror("{m,re}alloc failed");
			exit(1);
		}

		(*names)[*num_names] = strdup(name);
		if((*names)[*num_names] == NULL) {
			perror("strdup failed");
			exit(1);
		}
	}
	// {width}x{height}+{x}+{y}
	else if(sscanf(buf, "%ux%u+%u+%u", width, height, x, y) != 4) {
		return 0;
	}

	return 1;
}

static RLayout *
extract_layout(char *filename, int linenum, char *line)
{
	char **names = NULL, *geom;
	RAreaList *list;
	int num_names = 0;
	unsigned int width, height, x, y;

	list = RAreaListNew(10, NULL);
	names = NULL;

	while((geom = strsep(&line, " ")) != NULL) {
		if(geom[0] == '\0') {
			continue;
		}

		switch(extract_geometry(geom, &names, &num_names,
		                        &x, &y, &width, &height)) {
			case 1:
				break;

			case 0:
				fprintf(stderr, "%s:%d: layout unrecognized geometry (%s)\n",
				        filename, linenum, geom);
			// fallthrough
			default:
				free(names);
				return NULL;
		}

		RAreaListAdd(list, RAreaNewStatic((int)x, (int)y, (int)width, (int)height));
	}

	return RLayoutSetMonitorsNames(RLayoutNew(list), names);
}

static RLayout *
read_layout_file(FILE *file, char *filename)
{
	char buf[128], *line, **names;
	RAreaList *list;
	RLayout *layout;
	int num, num_names;
	bool comment = false;
	unsigned int width, height, x, y;

	list = RAreaListNew(10, NULL);
	names = NULL;
	num_names = 0;
	for(num = 1; fgets(buf, sizeof(buf), file) != NULL; num++) {
		line = trim_spaces(buf);

		// Multiline comments: =comment -> =end
		if(comment) {
			if(strcmp(line, "=end") == 0) {
				comment = false;
			}
			continue;
		}

		if(strcmp(line, "=comment") == 0) {
			comment = true;
			continue;
		}

		if(line[0] == '#' || line[0] == '\0') {
			continue;
		}

		if(strcmp(line, "__END__") == 0) {
			break;
		}

		switch(extract_geometry(line, &names, &num_names,
		                        &x, &y, &width, &height)) {
			case 1:
				break;

			case 0:
				fprintf(stderr, "%s:%d: layout unrecognized line (%s)\n", filename, num, line);
			// fallthrough
			default:
				free(names);
				return NULL;
		}

		RAreaListAdd(list, RAreaNewStatic((int)x, (int)y, (int)width, (int)height));
	}

	layout = RLayoutSetMonitorsNames(RLayoutNew(list), names);

	//RLayoutPrint(layout);

	return layout;
}

static int
read_test_from_file(char *filename)
{
	char buf[1024], *cur_buf, sub_buf[1024], func_buf[32], *line;
	FILE *file;
	RLayout *layout = NULL;
	RArea win = { 0 };
	int linenum, buf_size, expected1, expected2, errors;
	bool comment = false;
	unsigned int width, height, x, y;

	file = fopen(filename, "r");
	if(file == NULL) {
		fprintf(stderr, "Cannot open %s: %s\n", filename, strerror(errno));
		return 1;
	}

	errors = 0;

	cur_buf = buf;
	buf_size = sizeof(buf);

	for(linenum = 1; fgets(cur_buf, buf_size, file) != NULL; linenum++) {
		int last_chr_idx = strlen(cur_buf) - 1;
		if(last_chr_idx >= 0) {
			if(cur_buf[last_chr_idx] == '\n') {
				last_chr_idx--;
			}

			if(last_chr_idx >= 0 && cur_buf[last_chr_idx] == '\\') {
				cur_buf += last_chr_idx;
				buf_size -= last_chr_idx;

				if(buf_size > 0) {
					continue;
				}

				fprintf(stderr, "%s:%d: line too long\n", filename, linenum);
				errors++;
				break;
			}
		}

		cur_buf = buf;
		buf_size = sizeof(buf);

		line = trim_spaces(buf);

		// Multiline comments: =comment -> =end
		if(comment) {
			if(strcmp(line, "=end") == 0) {
				comment = false;
			}
			continue;
		}

		if(strcmp(line, "=comment") == 0) {
			comment = true;
			continue;
		}

		if(line[0] == '#' || line[0] == '\0') {
			continue;
		}

		// layout FILENAME|area ...
		if(sscanf(line, "layout %1023s", sub_buf) == 1) {
			FILE *file_layout;

			if((file_layout = fopen(sub_buf, "r")) != NULL) {
				layout = read_layout_file(file_layout, sub_buf);
				fclose(file_layout);
			}
			else if(errno == ENOENT) {
				line = trim_spaces(&line[7]);

				layout = extract_layout(filename, linenum, line);
			}

			if(layout != NULL) {
				continue;
			}

			fprintf(stderr, "%s:%d: layout error\n", filename, linenum);
			errors++;
			break;
		}

		// Gotta have a layout by now, right?
		assert(layout != NULL);

		// check_horizontal_layout area ...
		if(strncmp(line, "check_horizontal_layout ", 24) == 0) {
			RLayout *check_layout;

			line += 24;
			line = trim_spaces(line);

			check_layout = extract_layout(filename, linenum, line);
			if(check_layout == NULL) {
				break;
			}

			if(cmp_rarea_list(filename, linenum,
			                  "check_horizontal_layout",
			                  layout->horiz, check_layout->monitors)) {
				continue;
			}
			break;
		}

		// check_vertical_layout area ...
		if(strncmp(line, "check_vertical_layout ", 22) == 0) {
			RLayout *check_layout;

			line += 22;
			line = trim_spaces(line);

			check_layout = extract_layout(filename, linenum, line);
			if(check_layout == NULL) {
				break;
			}

			if(cmp_rarea_list(filename, linenum,
			                  "check_vertical_layout",
			                  layout->vert, check_layout->monitors)) {
				continue;
			}
			break;
		}

		// window area
		if(sscanf(line, "window %dx%d+%d+%d", &width, &height, &x, &y) == 4) {
			win = RAreaNew((int)x, (int)y, (int)width, (int)height);
			if(RAreaIsValid(&win)) {
				continue;
			}

			fprintf(stderr, "%s:%d: bad window geometry\n", filename, linenum);
			errors++;
			break;
		}

		if(layout == NULL) {
			fprintf(stderr,
			        "%s:%d: cannot continue as `layout ...' line not found\n",
			        filename, linenum);
			errors++;
			break;
		}

		if(!RAreaIsValid(&win)) {
			fprintf(stderr,
			        "%s:%d: cannot continue as `window ...' line not found\n",
			        filename, linenum);
			errors++;
			break;
		}

		// Function area
		//   RLayoutFull       area
		//   RLayoutFullHoriz  area
		//   RLayoutFullVert   area
		//   RLayoutFull1      area
		//   RLayoutFullHoriz1 area
		//   RLayoutFullVert1  area
		if(sscanf(line, "%31s %dx%d+%d+%d",
		                func_buf, &width, &height, &x, &y) == 5) {
			RArea got_area, expected_area;
			char *function = trim_prefix(func_buf, "RLayoutFull");

			expected_area = RAreaNew((int)x, (int)y, (int)width, (int)height);

			if(function[0] == '\0') {
				got_area = RLayoutFull(layout, &win);
			}
			else if(strcmp(function, "Horiz") == 0) {
				got_area = RLayoutFullHoriz(layout, &win);
			}
			else if(strcmp(function, "Vert") == 0) {
				got_area = RLayoutFullVert(layout, &win);
			}
			else if(strcmp(function, "1") == 0) {
				got_area = RLayoutFull1(layout, &win);
			}
			else if(strcmp(function, "Horiz1") == 0) {
				got_area = RLayoutFullHoriz1(layout, &win);
			}
			else if(strcmp(function, "Vert1") == 0) {
				got_area = RLayoutFullVert1(layout, &win);
			}
			else {
				fprintf(stderr, "%s:%d: bad function name bound to area\n",
				        filename, linenum);
				errors++;
				break;
			}

			if(memcmp(&got_area, &expected_area, sizeof(got_area)) != 0) {
				printf("%s:%d: %s failed\n"
				       "\t     got: %dx%d+%d+%d\n"
				       "\texpected: %dx%d+%d+%d\n",
				       filename, linenum, func_buf,
				       got_area.width, got_area.height, got_area.x, got_area.y,
				       expected_area.width, expected_area.height,
				       expected_area.x, expected_area.y);
				errors++;
			}
		}
		// Function num1 num2
		//   RLayoutFindTopBottomEdges top  bottom
		//   RLayoutFindLeftRightEdges left right
		else if(sscanf(line, "%31s %d %d",
		                func_buf, &expected1, &expected2) == 3) {
			int got1, got2;
			char *function = trim_prefix(func_buf, "RLayoutFind");

			if(strcmp(function, "TopBottomEdges") == 0) {
				RLayoutFindTopBottomEdges(layout, &win, &got1, &got2);
			}
			else if(strcmp(function, "LeftRightEdges") == 0) {
				RLayoutFindLeftRightEdges(layout, &win, &got1, &got2);
			}
			else {
				fprintf(stderr,
				        "%s:%d: bad function name bound to 2 expected results\n",
				        filename, linenum);
				errors++;
				break;
			}

			if(got1 != expected1 || got2 != expected2) {
				printf("%s:%d: %s failed\n"
				       "\t     got: (%d, %d)\n"
				       "\texpected: (%d, %d)\n",
				       filename, linenum, func_buf,
				       got1, got2,
				       expected1, expected2);
				errors++;
			}
		}
		// Function num
		//   RLayoutFindMonitorBottomEdge bottom
		//   RLayoutFindMonitorTopEdge    top
		//   RLayoutFindMonitorLeftEdge   left
		//   RLayoutFindMonitorRightEdge  right
		else if(sscanf(line, "%31s %d", func_buf, &expected1) == 2) {
			int got;
			char *function = trim_prefix(func_buf, "RLayoutFindMonitor");

			if(strcmp(function, "BottomEdge") == 0) {
				got = RLayoutFindMonitorBottomEdge(layout, &win);
			}
			else if(strcmp(function, "TopEdge") == 0) {
				got = RLayoutFindMonitorTopEdge(layout, &win);
			}
			else if(strcmp(function, "LeftEdge") == 0) {
				got = RLayoutFindMonitorLeftEdge(layout, &win);
			}
			else if(strcmp(function, "RightEdge") == 0) {
				got = RLayoutFindMonitorRightEdge(layout, &win);
			}
			else {
				fprintf(stderr,
				        "%s:%d: bad function name bound to one expected result\n",
				        filename, linenum);
				errors++;
				break;
			}

			if(got != expected1) {
				printf("%s:%d: %s failed\n"
				       "\t     got: %d\n"
				       "\texpected: %d\n",
				       filename, linenum, func_buf,
				       got,
				       expected1);
				errors++;
			}
		}
		else {
			fprintf(stderr, "%s:%d: test unrecognized line\n", filename, linenum);
			errors++;
			break;
		}
	}

	fclose(file);

	return errors;
}

int
main(int argc, char **argv)
{
	int i, error;

	if(argc < 2) {
		fprintf(stderr, "usage: %s TEST_FILE ...\n", argv[0]);
		return 1;
	}

	error = 0;
	for(i = 1; i < argc; i++) {
		printf("Testing %s\n", argv[i]);
		error = read_test_from_file(argv[i]) || error;
	}

	return error;
}

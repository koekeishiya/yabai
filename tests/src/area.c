struct test_area
{
    struct area area;
    CGPoint area_max;
};

static inline void init_test_display_list(struct test_area display_list[3])
{
    display_list[0].area.x   = 0;
    display_list[0].area.y   = 0;
    display_list[0].area.w   = 2560;
    display_list[0].area.h   = 1440;
    display_list[0].area_max = area_max_point(display_list[0].area);

    display_list[1].area.x   = -1728;
    display_list[1].area.y   = 0;
    display_list[1].area.w   = 1728;
    display_list[1].area.h   = 1117;
    display_list[1].area_max = area_max_point(display_list[1].area);

    display_list[2].area.x   = 2560;
    display_list[2].area.y   = 0;
    display_list[2].area.w   = 1920;
    display_list[2].area.h   = 1080;
    display_list[2].area_max = area_max_point(display_list[2].area);
}

TEST_FUNC(display_area_is_in_direction,
{
    struct test_area display_list[3];
    init_test_display_list(display_list);

    bool t1 = area_is_in_direction(&display_list[0].area, display_list[0].area_max, &display_list[1].area, display_list[1].area_max, DIR_WEST);
    TEST_CHECK(t1, true);

    bool t2 = area_is_in_direction(&display_list[0].area, display_list[0].area_max, &display_list[1].area, display_list[1].area_max, DIR_EAST);
    TEST_CHECK(t2, false);

    bool t3 = area_is_in_direction(&display_list[0].area, display_list[0].area_max, &display_list[2].area, display_list[2].area_max, DIR_WEST);
    TEST_CHECK(t3, false);

    bool t4 = area_is_in_direction(&display_list[0].area, display_list[0].area_max, &display_list[2].area, display_list[2].area_max, DIR_EAST);
    TEST_CHECK(t4, true);
});

static inline int closest_display_in_direction(struct test_area *display_list, int display_count, int source, int direction)
{
    int best_index    = -1;
    int best_distance = INT_MAX;

    for (int i = 0; i < display_count; ++i) {
        if (i == source) continue;

        if (area_is_in_direction(&display_list[source].area, display_list[source].area_max, &display_list[i].area, display_list[i].area_max, direction)) {
            int distance = area_distance_in_direction(&display_list[source].area, display_list[source].area_max, &display_list[i].area, display_list[i].area_max, direction);
            if (distance < best_distance) {
                best_index = i;
                best_distance = distance;
            }
        }
    }

    return best_index;
}

TEST_FUNC(closest_display_in_direction,
{
    int best_index;
    struct test_area display_list[3];
    init_test_display_list(display_list);

    best_index = closest_display_in_direction(display_list, array_count(display_list), 0, DIR_WEST);
    TEST_CHECK(best_index, 1);

    best_index = closest_display_in_direction(display_list, array_count(display_list), 1, DIR_WEST);
    TEST_CHECK(best_index, -1);

    best_index = closest_display_in_direction(display_list, array_count(display_list), 2, DIR_WEST);
    TEST_CHECK(best_index, 0);

    best_index = closest_display_in_direction(display_list, array_count(display_list), 0, DIR_EAST);
    TEST_CHECK(best_index, 2);

    best_index = closest_display_in_direction(display_list, array_count(display_list), 1, DIR_EAST);
    TEST_CHECK(best_index, 0);

    best_index = closest_display_in_direction(display_list, array_count(display_list), 2, DIR_EAST);
    TEST_CHECK(best_index, -1);
});

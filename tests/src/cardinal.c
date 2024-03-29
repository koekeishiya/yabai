TEST_FUNC(display_area_is_in_direction, {
    struct test_display {
        struct area area;
        CGPoint area_max;
    } displays[3];

    displays[0].area.x     = 0;
    displays[0].area.y     = 0;
    displays[0].area.w     = 2560;
    displays[0].area.h     = 1440;
    displays[0].area_max.x = displays[0].area.x + displays[0].area.w - 1;
    displays[0].area_max.y = displays[0].area.y + displays[0].area.h - 1;

    displays[1].area.x     = -1728;
    displays[1].area.y     = 0;
    displays[1].area.w     = 1728;
    displays[1].area.h     = 1117;
    displays[1].area_max.x = displays[1].area.x + displays[1].area.w - 1;
    displays[1].area_max.y = displays[1].area.y + displays[1].area.h - 1;

    displays[2].area.x     = 2560;
    displays[2].area.y     = 0;
    displays[2].area.w     = 1920;
    displays[2].area.h     = 1080;
    displays[2].area_max.x = displays[2].area.x + displays[2].area.w - 1;
    displays[2].area_max.y = displays[2].area.y + displays[2].area.h - 1;

    bool t1 = area_is_in_direction(&displays[0].area, displays[0].area_max, &displays[1].area, displays[1].area_max, DIR_WEST);
    result &= TEST_CHECK(t1, true);

    bool t2 = area_is_in_direction(&displays[0].area, displays[0].area_max, &displays[2].area, displays[2].area_max, DIR_WEST);
    result &= TEST_CHECK(t2, false);
});

TEST_FUNC(display_area_distance_in_direction, {
    struct test_display {
        struct area area;
        CGPoint area_max;
    } displays[3];

    displays[0].area.x     = 0;
    displays[0].area.y     = 0;
    displays[0].area.w     = 2560;
    displays[0].area.h     = 1440;
    displays[0].area_max.x = displays[0].area.x + displays[0].area.w - 1;
    displays[0].area_max.y = displays[0].area.y + displays[0].area.h - 1;

    displays[1].area.x     = -1728;
    displays[1].area.y     = 0;
    displays[1].area.w     = 1728;
    displays[1].area.h     = 1117;
    displays[1].area_max.x = displays[1].area.x + displays[1].area.w - 1;
    displays[1].area_max.y = displays[1].area.y + displays[1].area.h - 1;

    displays[2].area.x     = 2560;
    displays[2].area.y     = 0;
    displays[2].area.w     = 1920;
    displays[2].area.h     = 1080;
    displays[2].area_max.x = displays[2].area.x + displays[2].area.w - 1;
    displays[2].area_max.y = displays[2].area.y + displays[2].area.h - 1;

    {
        int best_index = -1;
        int best_distance = INT_MAX;

        for (int i = 0; i < array_count(displays); ++i) {
            if (i == 0) continue;

            bool direction = area_is_in_direction(&displays[0].area, displays[0].area_max, &displays[i].area, displays[i].area_max, DIR_WEST);
            if (direction) {
                int distance = area_distance_in_direction(&displays[0].area, displays[0].area_max, &displays[i].area, displays[i].area_max, DIR_WEST);
                if (distance < best_distance) {
                    best_index = i;
                    best_distance = distance;
                }
            }
        }

        result &= TEST_CHECK(best_index, 1);
    }

    {
        int best_index = -1;
        int best_distance = INT_MAX;

        for (int i = 0; i < array_count(displays); ++i) {
            if (i == 1) continue;

            bool direction = area_is_in_direction(&displays[1].area, displays[1].area_max, &displays[i].area, displays[i].area_max, DIR_WEST);
            if (direction) {
                int distance = area_distance_in_direction(&displays[1].area, displays[1].area_max, &displays[i].area, displays[i].area_max, DIR_WEST);
                if (distance < best_distance) {
                    best_index = i;
                    best_distance = distance;
                }
            }
        }

        result &= TEST_CHECK(best_index, -1);
    }

    {
        int best_index = -1;
        int best_distance = INT_MAX;

        for (int i = 0; i < array_count(displays); ++i) {
            if (i == 2) continue;

            bool direction = area_is_in_direction(&displays[2].area, displays[2].area_max, &displays[i].area, displays[i].area_max, DIR_WEST);
            if (direction) {
                int distance = area_distance_in_direction(&displays[2].area, displays[2].area_max, &displays[i].area, displays[i].area_max, DIR_WEST);
                if (distance < best_distance) {
                    best_index = i;
                    best_distance = distance;
                }
            }
        }

        result &= TEST_CHECK(best_index, 0);
    }
});

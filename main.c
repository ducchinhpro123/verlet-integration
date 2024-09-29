#include <raylib.h>
#include <raymath.h>
#include <stdio.h>

#define MAX_OBJECTS 500
/* Apply to the ball some physical :) */
typedef struct VerletObject
{
    Vector2 current_position;
    Vector2 old_position;
    Vector2 acceleration;
    Color color;
    float size;
} VerletObject;

void update_position(VerletObject *verlet_object, float delta_time);
void accelerate(VerletObject *verlet_object, Vector2 acc);

void solve_collision(VerletObject *objects, int count)
{
    for (int i = 0; i < count; i++)
    {
        for (int j = i + 1; j < count; j++)
        {
            VerletObject *object1 = &objects[i];
            VerletObject *object2 = &objects[j];

            Vector2 collision_axis =
                Vector2Subtract(object1->current_position, object2->current_position);
            float dist = Vector2Length(collision_axis);
            float min_dist = object1->size + object2->size;

            if (dist < min_dist)
            {
                Vector2 n = Vector2Normalize(collision_axis);
                float delta = min_dist - dist;
                object1->current_position =
                    Vector2Add(object1->current_position, Vector2Scale(n, delta * 0.5f));
                object2->current_position =
                    Vector2Subtract(object2->current_position, Vector2Scale(n, delta * 0.5f));
            }
        }
    }
}

VerletObject generate_verlet_object()
{
    VerletObject verlet_object = {0};
    verlet_object.size = GetRandomValue(10, 30);
    verlet_object.color = CLITERAL(Color){
        GetRandomValue(0, 255),
        GetRandomValue(0, 255),
        GetRandomValue(0, 255),
            255
    };
    verlet_object.current_position =
        (Vector2){GetScreenWidth() / 2.0f + 200.0f, GetScreenHeight() / 2.0f};
    verlet_object.old_position = verlet_object.current_position;

    return verlet_object;
}

int main(void)
{
    const int window_width = 1600, window_height = 900;
    InitWindow(window_width, window_height, "Verlet Integration");
    SetTargetFPS(60);

    VerletObject verlet_objects[MAX_OBJECTS];
    // The circle that wrap the other smaller circles

    Vector2 circle_cover = {window_width / 2.0f, window_height / 2.0f};
    float circle_cover_radius = 400;

    float elapsed_time = 0.0f;
    int active_objects = 0;
    Vector2 gravity = {0.0f, 1000.0f};

    while (!WindowShouldClose())
    {
        float delta_time = GetFrameTime();
        elapsed_time += delta_time;

        if (elapsed_time >= 0.05f && active_objects < MAX_OBJECTS)
        {
            verlet_objects[active_objects] = generate_verlet_object();
            active_objects++;

            elapsed_time = 0;
        }

        for (int i = 0; i < MAX_OBJECTS; i++)
        {
            VerletObject *verlet_object = &verlet_objects[i];
            accelerate(verlet_object, gravity);
            update_position(verlet_object, delta_time);

            Vector2 to_obj = Vector2Subtract(verlet_object->current_position, circle_cover);
            float dist = Vector2Length(to_obj);  // Magnitude of to_obj
            if (dist > (circle_cover_radius - verlet_object->size))
            {
                Vector2 n = Vector2Normalize(to_obj);
                verlet_object->current_position = Vector2Add(
                    circle_cover, Vector2Scale(n, circle_cover_radius - verlet_object->size));
            }
        }

        BeginDrawing();
        ClearBackground(RAYWHITE);
        DrawCircleV(circle_cover, circle_cover_radius, BLACK);

        for (int i = 0; i < MAX_OBJECTS; i++)
        {
            DrawCircleV(verlet_objects[i].current_position, verlet_objects[i].size,
                        verlet_objects[i].color);
        }
        solve_collision(verlet_objects, active_objects);

        DrawText(TextFormat("FPS: %d", GetFPS()), 10, 30, 20, RED);

        EndDrawing();
    }
    CloseWindow();
    return 0;
}

void update_position(VerletObject *verlet_object, float delta_time)
{
    Vector2 velocity;
    velocity.x = verlet_object->current_position.x - verlet_object->old_position.x;
    velocity.y = verlet_object->current_position.y - verlet_object->old_position.y;

    // Save current position into old position
    verlet_object->old_position = verlet_object->current_position;

    // Perform Verlet Integration
    verlet_object->current_position.x = verlet_object->current_position.x + velocity.x +
                                        verlet_object->acceleration.x * delta_time * delta_time;

    verlet_object->current_position.y = verlet_object->current_position.y + velocity.y +
                                        verlet_object->acceleration.y * delta_time * delta_time;

    // Reset acceleration
    verlet_object->acceleration.x = 0;
    verlet_object->acceleration.y = 0;
}

void accelerate(VerletObject *verlet_object, Vector2 acc)
{
    verlet_object->acceleration.x += acc.x;
    verlet_object->acceleration.y += acc.y;
}

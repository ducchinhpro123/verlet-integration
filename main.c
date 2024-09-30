#include <raylib.h>
#include <raymath.h>
#include <time.h>
/*#include <stdio.h>*/

#define MAX_OBJECTS 1000
/* Apply to the ball some physical :) */

typedef struct BigCircle
{
    Vector2 pos;
    Color color;
    float radius;
} BigCircle;

typedef struct VerletObject
{
    Vector2 current_position;
    Vector2 old_position;
    Vector2 acceleration;
    Color color;
    float size;
    float phase;
} VerletObject;

void update_position(VerletObject *verlet_object, float delta_time);
void accelerate(VerletObject *verlet_object, Vector2 acc);
void solve_collision(VerletObject *objects, int count);
void solve_collision_verlet_circle(VerletObject *verlet_object, BigCircle big_circle);
VerletObject generate_verlet_object();

float global_time;
float circle_area_verlet = 0;

int main(void)
{
    float elapsed_time = 0.0f;
    int active_objects = 0;
    int verlet_objects_number = 0;
    float percentage_occupied = 0;
    const int window_width = 1600, window_height = 900;

    InitWindow(window_width, window_height, "Verlet Integration");
    SetTargetFPS(60);

    VerletObject verlet_objects[MAX_OBJECTS];

    Vector2 gravity = {0.0f, 500.0f};

    BigCircle big_circle = {0};
    big_circle.radius = 450;
    big_circle.pos = (Vector2){window_width / 2.0f, window_height / 2.0f};
    big_circle.color = BLACK;

    float circle_area_circle_cover = PI * big_circle.radius * big_circle.radius;

    while (!WindowShouldClose())
    {
        float delta_time = GetFrameTime();
        elapsed_time += delta_time;
        global_time += delta_time;

        // The total area of verlet objects that generate_verlet_object() generated
        percentage_occupied = (circle_area_verlet / circle_area_circle_cover) * 100;

        // Generate verlet objects
        if (elapsed_time > 0.01f && percentage_occupied < 95)  // 80%
        {
            if (active_objects < MAX_OBJECTS)
            {
                verlet_objects[active_objects] = generate_verlet_object();
                active_objects++;
                elapsed_time = 0;
                verlet_objects_number++;
            }
        }

        for (int i = 0; i < MAX_OBJECTS; i++)
        {
            VerletObject *verlet_object = &verlet_objects[i];

            solve_collision_verlet_circle(verlet_object, big_circle);
            update_position(verlet_object, delta_time);
            accelerate(verlet_object, gravity);
        }

        solve_collision(verlet_objects, active_objects);
        BeginDrawing();

        ClearBackground(RAYWHITE);

        DrawCircleV(big_circle.pos, big_circle.radius, BLACK);

        for (int i = 0; i < MAX_OBJECTS; i++)
        {
            DrawCircleV(verlet_objects[i].current_position, verlet_objects[i].size,
                        verlet_objects[i].color);
        }

        DrawText(TextFormat("FPS: %d", GetFPS()), 10, 30, 20, RED);
        DrawText(TextFormat("percentage: %.2f %", percentage_occupied), 10, 50, 20, RED);
        DrawText(TextFormat("verlet_objects_number: %d", verlet_objects_number), 10, 70, 20, RED);

        EndDrawing();
    }
    CloseWindow();
    return 0;
}

void solve_collision_verlet_circle(VerletObject *verlet_object, BigCircle big_circle)
{
    Vector2 to_obj = Vector2Subtract(verlet_object->current_position, big_circle.pos);
    float dist = Vector2Length(to_obj);  // Magnitude of to_obj
    if (dist > (float)(big_circle.radius - verlet_object->size))
    {
        Vector2 n = Vector2Normalize(to_obj);
        verlet_object->current_position =
            Vector2Add(big_circle.pos, Vector2Scale(n, big_circle.radius - verlet_object->size));
    }
}

void update_position(VerletObject *verlet_object, float delta_time)
{
    Vector2 velocity = Vector2Subtract(verlet_object->current_position, verlet_object->old_position);

    // Save current position into old position
    verlet_object->old_position = verlet_object->current_position;

    // Perform Verlet Integration
    verlet_object->current_position = Vector2Add(
        verlet_object->current_position,
        Vector2Add(velocity, Vector2Scale(verlet_object->acceleration, delta_time * delta_time)));

    // Reset acceleration
    verlet_object->acceleration = (Vector2){0, 0};
}

void accelerate(VerletObject *verlet_object, Vector2 acc)
{
    verlet_object->acceleration = Vector2Add(verlet_object->acceleration, acc);
}

void solve_collision(VerletObject *objects, int count)
{
    for (int i = 0; i < count; i++)
    {
        VerletObject *object1 = &objects[i];
        for (int j = i + 1; j < count; j++)
        {
            VerletObject *object2 = &objects[j];

            Vector2 collision_axis = Vector2Subtract(object1->current_position, object2->current_position);
            float dist = Vector2Length(collision_axis);
            float min_dist = object1->size + object2->size;

            if (dist < min_dist)
            {
                Vector2 n = Vector2Normalize(collision_axis);
                float delta = min_dist - dist;
                object1->current_position = Vector2Add(object1->current_position, Vector2Scale(n, delta * 0.5f));
                object2->current_position = Vector2Subtract(object2->current_position, Vector2Scale(n, delta * 0.5f));
            }
        }
    }
}

VerletObject generate_verlet_object()
{
    SetRandomSeed(GetTime()*1000.0);
    VerletObject verlet_object = {0};
    verlet_object.size = GetRandomValue(1, 20);
    /*verlet_object.size = 20;*/
    circle_area_verlet += PI * verlet_object.size * verlet_object.size;

    const float r = sinf(global_time);
    const float g = sinf(global_time + 0.33f * 2.0f * PI);
    const float b = sinf(global_time + 0.66f * 2.0f * PI);

    verlet_object.color = (Color){(unsigned char)(255.0f * r * r), (unsigned char)(255.0f * g * g),
                                  (unsigned char)(255.0f * b * b), 255.0f};

    verlet_object.phase = global_time;

    /*float oscillation = 50.0f * sinf(verlet_object.phase);*/

    verlet_object.current_position =
        (Vector2){GetScreenWidth() / 2.0f + 200.0f, GetScreenHeight() / 2.0f - 200.0f};
    verlet_object.old_position = verlet_object.current_position;

    return verlet_object;
}

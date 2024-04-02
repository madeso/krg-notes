// https://www.youtube.com/watch?v=KyhrqbfEgfA

void increase_fov_based_on_speed()
{
    // or more specifically
    // the dot product of players 2d velocity and the look direction of the camera
    const auto dir = vec2_from(state->cam.dir);
    const auto proj = vec2_posproj(state->player->vel, dir) / 8.0f
    const auto norm = vec2_norm(proj);
    const auto value = saturate(norm);
    fov_out_target = ease_quat_in(value);
}

void tilt_effect()
{
    // applied when the player is moving in a direction perpendicular to the current view vector
    const vec2s vel_rel = vec2_scale(rotate(state->player->vel, state->cam.yaw - PI_2), -1);

    const f32 tilt_max = 0.16f + (state->cam.slide * 0.05f);
    tilt_target = tilt_max *
        clamp(
            (vel_rel.x / 14.0f) - (delta_yaw_tilt * 2.0f),
            -1.0f, 1.0f);
    tilt = dtlerp(tilt, tilt_target, 50.0f, state->time.dt_scaled);
}


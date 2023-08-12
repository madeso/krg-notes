```cpp
// data about how a single line of dialog should be played in the game
struct DialogLine
{
    // ID = <scene code><line number><character code>
    // good for last minute sanity check
    DialogId,
    Speaker;
    VolumeDb,
    Text,
    WaveFile,
    Emotion,
    CanBeInterrupted
};

// sort on area, scene
// csv export/import for google sheets
// import/export from google docs - everyone can comment?
// add note about character at the top with pictures

void SayLine(dialog_line_data)
{
    auto line_info - GetLineData(dialog_line_data.line_code);
    DisplayLine(line_info);

    auto speaker_entity = GetSpeakerEntity(line_info);
    speaker_entity.play_body_animation(line_info.GetBodyAnimation());
    speaker_entity.play_lipsynch_anim(line_info.GetLipsynchAnim());

    SoundManager.PlayVoiceLine(line_info);
}

// a collection of dialog lines, contains instructions about how the set of lines should be played
struct DialogSet
{
    struct Line
    {
        DialogId line_code;
        SetName set; // set within set
        float pre_line_pause_sec;
    };

    std::string name;
    std::vector<Line> contents;
    Speaker s;
    bool play_random; // weighted random
    // restriction on how often a set can be played?
    // priority: story > enviromental reaction
};

// launch dialog set on internal event (example: falling, recieved damage, dying)

```

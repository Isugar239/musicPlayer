
#define MAX_FILEPATH_RECORDED 4096
#define MAX_FILEPATH_SIZE 2048
#include <iostream>
#include <vector>
#include <fstream>
#include <cmath>
#include <cstdlib>
#include <SQLiteCpp/SQLiteCpp.h>
#include <SQLiteCpp/VariadicBind.h>
#include <filesystem>
#define RAYGUI_IMPLEMENTATION
#include <raygui.h>
#include <math.h>
// #include "FileReader.h"
using namespace SQLite;
using namespace std;

#include "helper.h"
#include "main.h"
static float averageVolume[120] = {0.0f};
struct Track
{
    unsigned int id;
    string name;
};

string readGIF(SQLite::Database &db, int gifId)
{
    cerr << "start reading gif" << "\n";
    SQLite::Statement query(db, "SELECT image FROM images WHERE id = ?");
    query.bind(1, gifId);
    string gifka = "";
    if (query.executeStep())
    {
        // cerr << query.getColumn(0);
        gifka += query.getColumn(0).getString();
    }
    query.reset();
    cerr << "stop reading gif" << "\n";

    cerr << gifka.size();
    return gifka;
}
string readPNG(SQLite::Database &db, int gifId)
{
    cerr << "start reading png" << "\n";
    SQLite::Statement query(db, "SELECT image FROM images WHERE id = ?");
    query.bind(1, gifId);
    string gifka = "";
    if (query.executeStep())
    {
        // cerr << query.getColumn(0);
        gifka += query.getColumn(0).getString();
    }
    query.reset();
    cerr << "stop reading gif" << "\n";

    cerr << gifka.size();
    return gifka;
}
string getTrackNames(const vector<Track> &tracks)
{
    string names = "";
    for (auto &t : tracks)
    {
        names += t.name + ";";
    }
    names[names.size() - 1] = '\0';
    return names;
}

vector<Track> getTracks(Database &db)
{
    SQLite::Statement query(db, "SELECT id, name_music FROM music");
    vector<Track> tempTracks;
    while (query.executeStep())
    {
        // Demonstrate how to get some typed column value
        int id = query.getColumn(0);
        string name = query.getColumn(1).getString();

        std::cout << "row: " << id << ", " << name << std::endl;

        tempTracks.push_back({(unsigned int)id, name});
    }
    query.reset();
    return tempTracks;
}
void ProcessAudio(void *buffer, unsigned int frames)
{
    float *samples = (float *)buffer; // Samples internally stored as <float>s
    float average = 0.0f;             // Temporary average volume

    for (unsigned int frame = 0; frame < frames; frame++)
    {
        float *left = &samples[frame * 2 + 0], *right = &samples[frame * 2 + 1];

        *left = powf(fabsf(*left), 1) * ((*left < 0.0f) ? -1.0f : 1.0f);
        *right = powf(fabsf(*right), 1) * ((*right < 0.0f) ? -1.0f : 1.0f);

        average += fabsf(*left) / frames; // accumulating average volume
        average += fabsf(*right) / frames;
    }

    // Moving history to the left
    for (int i = 0; i < 119; i++)
        averageVolume[i] = averageVolume[i + 1];

    averageVolume[119] = average; // Adding last average value
}
unsigned int findTrackId(const std::string &trackName, const std::vector<Track> &tracks)
{
    for (const auto &track : tracks)
    {
        if (track.name == trackName)
        {
            return track.id;
        }
    }
    throw std::runtime_error("Track not found");
}

void InserMusicBinary(SQLite::Database &db, std::string &MusTex, string nameoftrack)
{
    Transaction transaction(db);
    Statement insertMusicQuery{db, "INSERT INTO music(music, name_music) VALUES (?, ?)"};
    insertMusicQuery.bind(1, MusTex.c_str(), MusTex.size());
    insertMusicQuery.bind(2, nameoftrack);
    insertMusicQuery.exec();
    insertMusicQuery.reset();

    transaction.commit();
}

string *ReadMusicContent(int id, SQLite::Database &db)
{
    string *musicwave = new string();
    Statement musget(db, "SELECT music FROM music WHERE id==?");
    cout << "Reading content by id " << id << '\n';
    musget.bind(1, id);
    while (musget.executeStep())
    {
        *musicwave += musget.getColumn(0).getString();
        // cerr << musget.getColumn(0).getString().c_str() << endl;
    }
    // cout << hex << musicwave;
    // cout << dec;
    musget.reset();
    return musicwave;
}
void updateTrack(int &nowTrack, int &DropdownBox006Active, int &speedTrack, SQLite::Database &db, Music &nowMusic, int &act)
{
    nowTrack = DropdownBox006Active + 1;
    speedTrack = 1;
    string *musics = ReadMusicContent(nowTrack, db);
    nowMusic = LoadMusicStreamFromMemory(".wav", (unsigned char *)musics->c_str(), musics->size());

    if (!IsMusicValid(nowMusic))
    {
        cerr << "cannot read music content cause:\n";
        DropdownBox006Active -= 1;
        nowTrack -= 1;
        act = 1;
    }
    nowMusic.looping = false;
    cerr << "track" << nowTrack << endl;
}
void LoadFilepathToSQL(const char *filepaths, SQLite::Database &db)
{
    string MusTex;
    ifstream in{filepaths, std::ios::binary};
    char buf[409600];

    while (!in.eof())
    {
        in.read(buf, 409600);
        cout << in.gcount() << '\n';
        MusTex += string(buf, in.gcount());
    }
    // cerr << MusTex;
    in.close();
    std::filesystem::path p(filepaths);
    string nametrack = p.stem().string();

    InserMusicBinary(db, MusTex, nametrack);
}
std::string WStringToString(const std::wstring &wstr)
{
    std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
    return converter.to_bytes(wstr);
}
int main()
{
    int musicCounter = 0;
    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = 1600;
    const int screenHeight = 1000;
    InitWindow(screenWidth, screenHeight, "SoundOverFlow");
    // LoadFont("C:\\Users\\zakharoviz.28\\Downloads\\GenericMobileSystemNuevo.ttf");
    GuiLoadStyle(".\\style_ashes.rgs");
    // init music player---------------------------------------------------------------------
    InitAudioDevice();
    SetMasterVolume(1);
    AttachAudioMixedProcessor(ProcessAudio);
    Sound file;
    SetTargetFPS(60);
    
    while (!IsAudioDeviceReady())
    {
        cout << ".";
    }

    char *filePaths[MAX_FILEPATH_RECORDED] = {0}; // We will register a maximum of filepaths

    // Allocate space for the required file paths
    for (int i = 0; i < MAX_FILEPATH_RECORDED; i++)
    {
        filePaths[i] = (char *)RL_CALLOC(MAX_FILEPATH_SIZE, 1);
    }

    // init layouts------------------------------------------------------------------------------

    vector<wstring> winapifiles;

    static float *delayBuffer = NULL;
    static unsigned int delayBufferSize = 48000 * 2; // 1 second delay (device sampleRate*channels)
    delayBuffer = (float *)RL_CALLOC(delayBufferSize, sizeof(float));
    static unsigned int delayReadIndex = 2;
    static unsigned int delayWriteIndex = 0;
    bool IsPlaying = false;
    float SliderBar004Value = 0.0f;
    float ProgressBar005Value = 0.0f;
    int ToggleGroup006Active = 1;
    bool DropdownBox006EditMode = false;
    int DropdownBox006Active = 1;
    int speed = 10;
    float nullspeed = 0;
    float dt = 0;
    float prevTime = 0;
    bool mir = false;
    Music nowMusic;
    string nametrack;
    int action = 1;
    bool AP = false;
    int nowTrack = -1;
    int speedTrack = 1;
    float musicLength = 0;
    float masterVol = 1;
    int currentAnimFrame = 0; // Current animation frame to load and draw
    int frameDelay = 8;
    int frameCounter = 0;

    Database db("music.db3", SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE);
    db.exec(R"(
    CREATE TABLE IF NOT EXISTS music (
        id INTEGER PRIMARY KEY AUTOINCREMENT,
        name_music TEXT,
        music BLOB
    );
    CREATE TABLE IF NOT EXISTS images (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    name TEXT,
    image BLOB
    );
)");

    vector<Track> tracks;
    tracks = getTracks(db);
    double ATimer = GetTime();
    string png = readPNG(db, 2);
    Image DI = LoadImageFromMemory(".png", (unsigned char*)png.c_str(), png.size());
    ImageResize(&DI, 200, 200);
    Texture2D DropImage = LoadTextureFromImage(DI);
    string gif = readGIF(db, 1);
    int animFrames;
    Image imgsof = LoadImageAnimFromMemory(".gif", (unsigned char *)gif.c_str(), gif.size(), &animFrames);
    Texture2D SOF = LoadTextureFromImage(imgsof);
    while (!WindowShouldClose())
    {
        dt = GetTime() - prevTime;
        prevTime = GetTime();
        nullspeed = 1000.0 / 16 * dt;
        frameCounter++;
        if (frameCounter >= frameDelay)
        {
            // Move to next frame
            // NOTE: If final frame is reached we return to first frame
            currentAnimFrame++;
            if (currentAnimFrame >= animFrames)
                currentAnimFrame = 0;

            // Get memory offset position for next frame data in image.data
            int nextFrameDataOffset = imgsof.width * imgsof.height * 4 * currentAnimFrame;

            // Update GPU texture data with next frame image data
            // WARNING: Data size (frame size) and pixel format must match already created texture
            UpdateTexture(SOF, ((unsigned char *)imgsof.data) + nextFrameDataOffset);

            frameCounter = 0;
        }

        if (IsFileDropped())
        {
            if (CheckCollisionPointRec(GetMousePosition(), (Rectangle){1100, 100, 450, 800}))
            {
                FilePathList droppedFiles = LoadDroppedFiles();
                cerr << droppedFiles.paths[0];
                LoadFilepathToSQL(droppedFiles.paths[0], db);

                tracks = getTracks(db);
                UnloadDroppedFiles(droppedFiles); // Unload filepaths from memory
            }
            else
            {
                FilePathList droppedFiles = LoadDroppedFiles();
                UnloadDroppedFiles(droppedFiles); // Unload filepaths from memory
            }
        }

        if (nowTrack != DropdownBox006Active + 1)
        {
            ToggleGroup006Active = 1;
            updateTrack(nowTrack, DropdownBox006Active, speedTrack, db, nowMusic, ToggleGroup006Active);
        }
        if (ToggleGroup006Active != action)
        {
            action = ToggleGroup006Active;
            cerr << "act" << action << endl;
        }

        if (IsPlaying && !IsMusicStreamPlaying(nowMusic) && action == 2)
        {
            ProgressBar005Value = 0.0f;
            if (AP)
            {
                DropdownBox006Active++;
                cerr << "autoPlaying: " << DropdownBox006Active;
                IsPlaying = false;
                ToggleGroup006Active = 2;
                action = 2;
                updateTrack(nowTrack, DropdownBox006Active, speedTrack, db, nowMusic, ToggleGroup006Active);
                action = ToggleGroup006Active;
            }
            else
            {
                ToggleGroup006Active = 1;
            }
        }
        if (action == 1 && IsPlaying)
        {
            PauseMusicStream(nowMusic);
            IsPlaying = false;
            cerr << "stop playing\n";
            cerr << ProgressBar005Value;
        }
        if (action == 0)
        {

            SeekMusicStream(nowMusic, 0);
            ProgressBar005Value = 0.0f;
            action = 1;
            ToggleGroup006Active = 1;
        }
        if (action == 3)
        {
            DropdownBox006Active++;
        }
        if ((!IsPlaying) && (action == 2))
        {

            IsPlaying = true;
            musicLength = GetMusicTimeLength(nowMusic);
            if (GetMusicTimePlayed(nowMusic) > 0)
            {
                ResumeMusicStream(nowMusic);
                cerr << "resume playing";
            }
            else
            {
                cerr << "start playing";

                PlayMusicStream(nowMusic);
            }
        }

        if (IsMusicValid(nowMusic) && action == 2)
        {
            float playedTime = GetMusicTimePlayed(nowMusic);

            if (!IsMusicStreamPlaying(nowMusic) && playedTime < 0.01f)
            {
                ProgressBar005Value = 0.0f;
            }
            else
            {
                ProgressBar005Value = playedTime / musicLength;
            }

            UpdateMusicStream(nowMusic);
        }
        SetMasterVolume(masterVol);
        SetMusicPitch(nowMusic, (float)speedTrack);
        BeginDrawing();
        ClearBackground(GetColor(GuiGetStyle(DEFAULT, BACKGROUND_COLOR)));

        // raygui: controls drawing
        //----------------------------------------------------------------------------------
        if (DropdownBox006EditMode)
            GuiLock();
        GuiGroupBox((Rectangle){0, 5, 1600, 1000}, "Music player");
        if (GuiButton((Rectangle){37, 160, 120, 16}, "#05#Open music"))
        {
            OpenFileDialog(winapifiles);
            LoadFilepathToSQL(WStringToString(winapifiles.back()).c_str(), db);
            tracks = getTracks(db);
            winapifiles.clear();
        }

        // SliderBar004Value = GuiSliderBar(layoutRecs[2], NULL, NULL, SliderBar004Value, 0, 100);
        GuiProgressBar((Rectangle){37, 585, 120, 16}, NULL, NULL, &ProgressBar005Value, 0, 1);
        GuiLabel((Rectangle){37, 106, 120, 10}, "Master volume");
        GuiSlider((Rectangle){37, 120, 120, 24}, NULL, NULL, &masterVol, 0, 1);
        GuiToggleGroup((Rectangle){37, 610, 28, 28}, "#129#;#132#;#131#;#134#", &ToggleGroup006Active);
        if (GuiDropdownBox((Rectangle){37, 191, 120, 23}, getTrackNames(tracks).c_str(), &DropdownBox006Active, DropdownBox006EditMode))
            DropdownBox006EditMode = !DropdownBox006EditMode;
        GuiSpinner((Rectangle){37, 650, 120, 16}, "speed", &speedTrack, 1, 4, false);
        GuiCheckBox((Rectangle){37, 500, 120, 16}, "AutoPlay", &AP);
        GuiCheckBox((Rectangle){37, 520, 120, 16}, "Looping", &nowMusic.looping);
        DrawTexture(SOF, 400, 400, WHITE);
        DrawRectangleRounded((Rectangle){1100, 100, 450, 800}, 0.2, 150, WHITE);
        DrawRectangleRounded((Rectangle){1080, 80, 490, 840}, 0.2, 150, Color{255, 255, 255, 120});
        DrawRectangleLines(37, 420, 120, 60, BLACK);
        for (int i = 0; i < 120; i++)
        {
            DrawLine(37 + i, 450 - (int)(averageVolume[i] * 32), 37 + i, 450, BLUE);
        }
        for (int i = 0; i < 120; i++)
        {
            DrawLine(37 + i, 450, 37 + i, 450 + (int)(averageVolume[i] * 32), BLUE);
        }
        DrawTexture(DropImage, 1225, 400, BLACK);
        GuiUnlock();

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    CloseWindow(); // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}
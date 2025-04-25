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
using namespace SQLite;
using namespace std;

#include "helper.h"
static float averageVolume[120] = {0.0};
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
        gifka += query.getColumn(0).getString();
    }
    query.reset();
    cerr << "stop reading gif" << "\n";

    cerr << gifka.size();
    return gifka;
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
    in.close();
    std::filesystem::path p(filepaths);
    string NowNameTrack = p.stem().string();

    InserMusicBinary(db, MusTex, NowNameTrack);
}
vector<Track> getTracks(Database &db)
{
    SQLite::Statement query(db, "SELECT id, name_music FROM music");
    vector<Track> tempTracks;
    while (query.executeStep())
    {
        int id = query.getColumn(0);
        string name = query.getColumn(1).getString();

        std::cout << "row: " << id << ", " << name << std::endl;

        tempTracks.push_back({(unsigned int)id, name});
    }
    query.reset();
    return tempTracks;
}
std::string WStringToString(const std::wstring &wstr)
{
    std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
    return converter.to_bytes(wstr);
}
void DrawAll(bool &DropdownBox006EditMode, std::vector<std::wstring> &winapifiles, SQLite::Database &db, std::vector<Track> &tracks, float &ProgressBar005Value, float &masterVol, int &ToggleGroup006Active, int &DropdownBox006Active, int &speedTrack, bool &IsAutoPlay, Music &nowMusic, const Texture2D &SOF, const Texture2D &DropImage)
{
    ClearBackground(GetColor(GuiGetStyle(DEFAULT, BACKGROUND_COLOR)));

    if (DropdownBox006EditMode)
        GuiLock();
    if (GuiButton((Rectangle){37, 160, 120, 16}, "#05#Open music"))
    {
        OpenFileDialog(winapifiles);
        string s = WStringToString(winapifiles.back());
        LoadFilepathToSQL(s.c_str(), db);
        tracks = getTracks(db);
        winapifiles.clear();
    }
    GuiGroupBox((Rectangle){0, 5, 1600, 1000}, "Music player");
    GuiProgressBar((Rectangle){37, 585, 120, 16}, NULL, NULL, &ProgressBar005Value, 0, 1);
    GuiLabel((Rectangle){37, 106, 120, 10}, "Master volume");
    GuiSlider((Rectangle){37, 120, 120, 24}, NULL, NULL, &masterVol, 0, 1);
    GuiToggleGroup((Rectangle){37, 610, 28, 28}, "#129#;#132#;#131#;#134#", &ToggleGroup006Active);
    if (GuiDropdownBox((Rectangle){37, 191, 120, 23}, getTrackNames(tracks).c_str(), &DropdownBox006Active, DropdownBox006EditMode))
        DropdownBox006EditMode = !DropdownBox006EditMode;
    GuiSpinner((Rectangle){37, 650, 120, 16}, "speed", &speedTrack, 1, 4, false);
    GuiCheckBox((Rectangle){37, 500, 120, 16}, "AutoPlay", &IsAutoPlay);
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
}
void ProcessAudio(void *buffer, unsigned int frames)
{
    float *samples = (float *)buffer;
    float average = 0.0;

    for (unsigned int frame = 0; frame < frames; frame++)
    {
        float *left = &samples[frame * 2 + 0], *right = &samples[frame * 2 + 1];

        *left = powf(fabsf(*left), 1) * ((*left < 0.0) ? -1.0 : 1.0);
        *right = powf(fabsf(*right), 1) * ((*right < 0.0) ? -1.0 : 1.0);

        average += fabsf(*left) / frames; // accumulating average volume
        average += fabsf(*right) / frames;
    }

    for (int i = 0; i < 119; i++)
        averageVolume[i] = averageVolume[i + 1];

    averageVolume[119] = average;
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

string *ReadMusicContent(int id, SQLite::Database &db)
{
    string *musicwave = new string();
    Statement musget(db, "SELECT music FROM music WHERE id==?");
    cout << "Reading content by id " << id << '\n';
    musget.bind(1, id);
    while (musget.executeStep())
    {
        *musicwave += musget.getColumn(0).getString();
    }
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
int main()
{
    const int screenWidth = 1600;
    const int screenHeight = 1000;
    InitWindow(screenWidth, screenHeight, "SoundOverFlow");
    GuiLoadStyle(".\\style_ashes.rgs");
    InitAudioDevice();
    SetMasterVolume(1);
    AttachAudioMixedProcessor(ProcessAudio);
    SetTargetFPS(60);
    while (!IsAudioDeviceReady())
    {
        cout << ".";
    }
    char *filePaths[MAX_FILEPATH_RECORDED] = {0};
    for (int i = 0; i < MAX_FILEPATH_RECORDED; i++)
    {
        filePaths[i] = (char *)RL_CALLOC(MAX_FILEPATH_SIZE, 1);
    }

    int musicCounter = 0;
    static float *delayBuffer = NULL;
    static unsigned int delayBufferSize = 44100 * 2; //(sampleRate*channels)
    delayBuffer = (float *)RL_CALLOC(delayBufferSize, sizeof(float));
    static unsigned int delayReadIndex = 2;
    static unsigned int delayWriteIndex = 0;
    bool IsPlaying = false;
    float SliderBar004Value = 0.0;
    float ProgressBar005Value = 0.0;
    int ToggleGroup006Active = 1;
    bool DropdownBox006EditMode = false;
    int DropdownBox006Active = 1;
    Music nowMusic;
    string NowNameTrack;
    vector<Track> tracks;
    vector<wstring> winapifiles;
    int action = 1;
    bool IsAutoPlay = false;
    int nowTrack = -1;
    int speedTrack = 1;
    float musicLength = 0;
    float masterVol = 1;
    int currentAnimFrame = 0;
    int frameDelay = 8;
    int frameCounter = 0;
    int animFrames;

    Database db("music.db3", SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE);
    db.exec(R"(CREATE TABLE IF NOT EXISTS music (id INTEGER PRIMARY KEY AUTOINCREMENT,name_music TEXT,music BLOB);
    CREATE TABLE IF NOT EXISTS images (id INTEGER PRIMARY KEY AUTOINCREMENT,name TEXT,image BLOB);)");
    tracks = getTracks(db);
    string png = readPNG(db, 2);
    Image DownloadedImage = LoadImageFromMemory(".png", (unsigned char *)png.c_str(), png.size());
    ImageResize(&DownloadedImage, 200, 200);
    Texture2D DropImage = LoadTextureFromImage(DownloadedImage);
    string gif = readGIF(db, 1);
    Image imgsof = LoadImageAnimFromMemory(".gif", (unsigned char *)gif.c_str(), gif.size(), &animFrames);
    Texture2D SOF = LoadTextureFromImage(imgsof);
    double ATimer = GetTime();
    while (!WindowShouldClose())
    {
        frameCounter++;
        if (frameCounter >= frameDelay)
        {
            currentAnimFrame++;
            if (currentAnimFrame >= animFrames)
                currentAnimFrame = 0;
            int nextFrameDataOffset = imgsof.width * imgsof.height * 4 * currentAnimFrame;
            UpdateTexture(SOF, ((unsigned char *)imgsof.data) + nextFrameDataOffset);
            frameCounter = 0;
        }

        if (IsFileDropped())
        {
            FilePathList droppedFiles = LoadDroppedFiles();
            if (CheckCollisionPointRec(GetMousePosition(), (Rectangle){1100, 100, 450, 800}))
            {
                LoadFilepathToSQL(droppedFiles.paths[0], db);
                tracks = getTracks(db);
            }
            UnloadDroppedFiles(droppedFiles);
        }
        if (nowTrack != DropdownBox006Active + 1)
        {
            ToggleGroup006Active = 1;
            updateTrack(nowTrack, DropdownBox006Active, speedTrack, db, nowMusic, ToggleGroup006Active);
        }
        if (ToggleGroup006Active != action)
        {
            action = ToggleGroup006Active;
        }
        if (IsPlaying && !IsMusicStreamPlaying(nowMusic) && action == 2)
        {
            ProgressBar005Value = 0.0;
            if (IsAutoPlay)
            {
                DropdownBox006Active++;
                IsPlaying = false;
                ToggleGroup006Active = 2;
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
        }
        if (action == 0)
        {

            SeekMusicStream(nowMusic, 0);
            ProgressBar005Value = 0.0;
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
            }
            else
            {
                PlayMusicStream(nowMusic);
            }
        }

        if (IsMusicValid(nowMusic) && action == 2)
        {
            float playedTime = GetMusicTimePlayed(nowMusic);

            if (!IsMusicStreamPlaying(nowMusic) && playedTime < 0.01f)
            {
                ProgressBar005Value = 0.0;
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
        DrawAll(DropdownBox006EditMode, winapifiles, db, tracks, ProgressBar005Value, masterVol, ToggleGroup006Active, DropdownBox006Active, speedTrack, IsAutoPlay, nowMusic, SOF, DropImage);

        EndDrawing();
    }

    CloseWindow();
    return 0;
}

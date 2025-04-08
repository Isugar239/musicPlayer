
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

// #include "FileReader.h"
using namespace SQLite;
using namespace std;

#include "helper.h"
#include "main.h"

struct Track
{
    unsigned int id;
    string name;
};

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
    GuiLoadStyle("C:\\Users\\zakharoviz.28\\Downloads\\style_enefete.rgs");
    // init music player---------------------------------------------------------------------
    InitAudioDevice();
    SetMasterVolume(1);
    Sound file;
    SetTargetFPS(360);
    Image DI = LoadImage("send.png");
    ImageResize(&DI, 200, 200);
    Texture2D DropImage = LoadTextureFromImage(DI);
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
    Rectangle layoutRecs[8] = {

        (Rectangle){16, 40, 160, 608},
        (Rectangle){0, 0, 1600, 1000},
        (Rectangle){30, 160, 120, 16},
        (Rectangle){37, 585, 120, 16},
        (Rectangle){27, 106, 120, 24},
        (Rectangle){34, 610, 40, 24},
        (Rectangle){40, 191, 113, 23},
        (Rectangle){27, 130, 120, 24},

    };
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
    int nowTrack = -1;
    float musicLength = 0;
    float masterVol = 1;
    Database db("music.db3", SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE);
    db.exec(R"(
    CREATE TABLE IF NOT EXISTS music (
        id INTEGER PRIMARY KEY AUTOINCREMENT,
        name_music TEXT,
        music BLOB
    );
)");

    vector<Track> tracks;
    tracks = getTracks(db);

    while (!WindowShouldClose())
    {
        dt = GetTime() - prevTime;
        prevTime = GetTime();
        nullspeed = 1000.0 / 16 * dt;
        if (GetMouseX() > 100 && GetMouseX() < 200 && GetMouseY() > 100 && GetMouseY() < 200)
        {
            // mir = true;
        }
        else
            mir = false;

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
            else{
                FilePathList droppedFiles = LoadDroppedFiles();
                UnloadDroppedFiles(droppedFiles); // Unload filepaths from memory
            }
        }
        if (nowTrack != DropdownBox006Active + 1)
        {
            nowTrack = DropdownBox006Active + 1;
            ToggleGroup006Active = 1;
            string *musics = ReadMusicContent(nowTrack, db);
            nowMusic = LoadMusicStreamFromMemory(".wav", (unsigned char *)musics->c_str(), musics->size());
            nowMusic.looping = false;
            cerr << "track" << nowTrack << endl;
        }
        if (ToggleGroup006Active != action)
        {
            action = ToggleGroup006Active;
            cerr << "act" << action << endl;
        }
        if (action == 1 && IsPlaying)
        {
            PauseMusicStream(nowMusic);
            IsPlaying = false;
            cerr << "stop playing";
        }
        if (action == 0)
        {

            SeekMusicStream(nowMusic, 0);
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
        if (IsMusicValid(nowMusic))
        {
            ProgressBar005Value = GetMusicTimePlayed(nowMusic) / musicLength;
            UpdateMusicStream(nowMusic);
        }
        SetMasterVolume(masterVol);
        BeginDrawing();
        ClearBackground(GetColor(GuiGetStyle(DEFAULT, BACKGROUND_COLOR)));

        // raygui: controls drawing
        //----------------------------------------------------------------------------------
        if (DropdownBox006EditMode)
            GuiLock();

        GuiGroupBox(layoutRecs[1], "Music player");
        if (GuiButton(layoutRecs[2], "#05#Open music"))
        {
            OpenFileDialog(winapifiles);
            LoadFilepathToSQL(WStringToString(winapifiles.back()).c_str(), db);
            tracks = getTracks(db);
            winapifiles.clear();
        }
        // SliderBar004Value = GuiSliderBar(layoutRecs[2], NULL, NULL, SliderBar004Value, 0, 100);
        GuiProgressBar(layoutRecs[3], NULL, NULL, &ProgressBar005Value, 0, 1);
        GuiLabel(layoutRecs[4], "Master volume");
        GuiSlider(layoutRecs[7], NULL, NULL, &masterVol, 0, 1);
        GuiToggleGroup(layoutRecs[5], "#129#;#132#;#131#;#134#", &ToggleGroup006Active);
        if (GuiDropdownBox(layoutRecs[6], getTrackNames(tracks).c_str(), &DropdownBox006Active, DropdownBox006EditMode))
            DropdownBox006EditMode = !DropdownBox006EditMode;
        DrawRectangleRounded((Rectangle){1100, 100, 450, 800}, 0.2, 150, WHITE);
        DrawRectangleRounded((Rectangle){1080, 80, 490, 840}, 0.2, 150, Color{255, 255, 255, 120});
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

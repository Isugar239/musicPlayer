#pragma once
void InserMusicBinary(SQLite::Database &db, std::string &MusTex, SQLite::Transaction &transaction);

void LoadFilepathToSQL(FilePathList &droppedFiles, SQLite::Database &db);

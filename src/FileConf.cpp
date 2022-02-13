// Copyright (C) 2022 Rafael Fassi Lobao
// This file is part of qlogexplorer project licensed under GPL-3.0

#include "pch.h"
#include "FileConf.h"

FileConf::FileConf(tp::FileType fileType) : m_fileType(fileType)
{
}

FileConf::FileConf(const std::string &confFileName)
{
    loadFConf(confFileName);
}

bool FileConf::loadFConf(const std::string &confFileName)
{
    std::ifstream ifs(confFileName);
    if (!ifs.is_open())
    {
        LOG_ERR("Unable to open {}", confFileName);
        return false;
    }

    rapidjson::IStreamWrapper isw(ifs);

    rapidjson::Document d;
    d.ParseStream(isw);
    if (d.HasParseError())
    {
        LOG_ERR("Error {} parsing config file '{}' at offset {}", d.GetParseError(), confFileName, d.GetErrorOffset());
        return false;
    }

    fromJson(d);

    m_confFileName = confFileName;

    return true;
}

void FileConf::saveConfAs(const std::string &confFileName)
{
    const auto &jDoc = toJson();
    rapidjson::StringBuffer buffer;
    rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);
    jDoc.Accept(writer);

    std::ofstream ofs(confFileName);
    if (!ofs.is_open())
    {
        LOG_ERR("Unable to open {}", confFileName);
        return;
    }

    ofs << buffer.GetString();
    ofs.flush();
    ofs.close();
    m_confFileName = confFileName;
}

void FileConf::saveConf()
{
    if (m_confFileName.empty())
    {
        LOG_ERR("No config file defined");
        return;
    }
    saveConfAs(m_confFileName);
}

void FileConf::fromJson(const rapidjson::Document &jDoc)
{
    m_configName = utl::GetValueOpt<std::string>(jDoc, "configName").value_or(std::string());
    m_fileType = utl::GetValueOpt<tp::FileType>(jDoc, "fileType").value_or(tp::FileType::Text);
    m_regexPattern = utl::GetValueOpt<std::string>(jDoc, "regexPattern").value_or(std::string());
    m_noMatchColumn = utl::GetValueOpt<tp::SInt>(jDoc, "noMatchColumn").value_or(0);

    if (const auto &colsIt = jDoc.FindMember("columns"); colsIt != jDoc.MemberEnd())
    {
        for (const auto &col : colsIt->value.GetArray())
        {
            tp::Column column;
            column.key = utl::GetValueOpt<std::string>(col, "key").value_or(std::string());
            column.name = utl::GetValueOpt<std::string>(col, "name").value_or(std::string());
            column.type = utl::GetValueOpt<tp::ColumnType>(col, "type").value_or(tp::ColumnType::Str);
            column.format = utl::GetValueOpt<std::string>(col, "format").value_or(std::string());
            column.width = utl::GetValueOpt<tp::SInt>(col, "width").value_or(-1L);
            column.pos = utl::GetValueOpt<tp::SInt>(col, "pos").value_or(-1L);
            m_columns.emplace_back(std::move(column));
        }
    }

    if (const auto &hltIt = jDoc.FindMember("highlighters"); hltIt != jDoc.MemberEnd())
    {
        for (const auto &hlt : hltIt->value.GetArray())
        {
            tp::HighlighterParam hParam;
            hParam.searchParam.column = utl::GetValueOpt<tp::UInt>(hlt, "column");
            hParam.searchParam.type = utl::GetValueOpt<tp::SearchType>(hlt, "type").value_or(tp::SearchType::SubString);
            hParam.searchParam.flags = utl::GetValueOpt<tp::SearchFlags>(hlt, "options").value_or(tp::SearchFlags());
            hParam.searchParam.pattern = utl::GetValueOpt<std::string>(hlt, "pattern").value_or(std::string());
            hParam.color.bg = utl::GetValueOpt<std::string>(hlt, "backColor").value_or("White").c_str();
            hParam.color.fg = utl::GetValueOpt<std::string>(hlt, "textColor").value_or("Black").c_str();
            m_highlighterParams.emplace_back(std::move(hParam));
        }
    }
}

rapidjson::Document FileConf::toJson() const
{
    rapidjson::Document jDoc(rapidjson::kObjectType);
    auto &alloc = jDoc.GetAllocator();

    jDoc.AddMember("configName", m_configName, alloc);
    jDoc.AddMember("fileType", tp::toStr(m_fileType), alloc);
    jDoc.AddMember("regexPattern", m_regexPattern, alloc);
    jDoc.AddMember("noMatchColumn", m_noMatchColumn, alloc);

    rapidjson::Value jCols(rapidjson::kArrayType);
    for (const auto &col : m_columns)
    {
        rapidjson::Value jCol(rapidjson::kObjectType);
        jCol.AddMember("pos", col.pos, alloc);
        jCol.AddMember("key", col.key, alloc);
        jCol.AddMember("name", col.name, alloc);
        jCol.AddMember("type", tp::toStr(col.type), alloc);
        jCol.AddMember("format", col.format, alloc);
        jCol.AddMember("width", col.width, alloc);
        jCols.GetArray().PushBack(jCol, alloc);
    }
    jDoc.AddMember("columns", jCols, alloc);

    rapidjson::Value jHighlighters(rapidjson::kArrayType);
    for (const auto &hlt : m_highlighterParams)
    {
        rapidjson::Value jHlt(rapidjson::kObjectType);
        jHlt.AddMember("textColor", utl::toStr(hlt.color.fg), alloc);
        jHlt.AddMember("backColor", utl::toStr(hlt.color.bg), alloc);
        jHlt.AddMember("type", tp::toStr(hlt.searchParam.type), alloc);
        jHlt.AddMember("options", tp::toStr(hlt.searchParam.flags), alloc);
        jHlt.AddMember("pattern", hlt.searchParam.pattern, alloc);
        if (hlt.searchParam.column.has_value())
            jHlt.AddMember("column", hlt.searchParam.column.value().idx, alloc);
        jHighlighters.GetArray().PushBack(jHlt, alloc);
    }
    jDoc.AddMember("highlighters", jHighlighters, alloc);

    return jDoc;
}

bool FileConf::hasDefinedColumn(tp::SInt columnIdx) const
{
    return (hasDefinedColumns() && (-1L < columnIdx) && (columnIdx < m_columns.size()));
}

bool FileConf::hasHighlighterParam(tp::SInt hltIdx) const
{
    return ((-1L < hltIdx) && (hltIdx < m_highlighterParams.size()));
}

bool FileConf::isEqual(const FileConf::Ptr &other) const
{
    return (*this == *other.get());
}

bool FileConf::isSameType(const FileConf::Ptr &other) const
{
    return ((getFileType() == other->getFileType()) && (getConfFileName() == other->getConfFileName()));
}

bool FileConf::isSameFileAndType(const FileConf::Ptr &other) const
{
    return (isSameType(other) && (getFileName() == other->getFileName()));
}

void FileConf::copyFrom(const FileConf::Ptr &other)
{
    *this = *other.get();
}

void FileConf::copyTypeFrom(const FileConf::Ptr &other)
{
    m_fileType = other->getFileType();
    m_confFileName = other->getConfFileName();
    m_configName = other->getConfigName();
}

void FileConf::copyFileAndTypeFrom(const FileConf::Ptr &other)
{
    copyTypeFrom(other);
    setFileName(other->getFileName());
}

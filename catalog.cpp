//
// Created by wjh on 2026/6/15.
//
#include"catalog.h"
#include<cassert>
minidb::TableMetaData::TableMetaData(Schema schema, std::unique_ptr<TableHeap> table_heap)
: schema_(std::move(schema)), table_heap_(std::move(table_heap))
{
}

minidb::Catalog::Catalog(BufferPoolManager *bpm)
    :bpm_(bpm)
{
    Page *meta_data_page = nullptr;
    if (bpm_->GetPageNumber()) {
        meta_data_page=bpm_->FetchPage(0);
        assert(meta_data_page!=nullptr);
        DeserializeFrom(meta_data_page->GetData());

    } else {
        page_id_t meta_data_id;
        meta_data_page=bpm_->NewPage(&meta_data_id);
        assert(meta_data_page&&meta_data_id==0);
    }

    bpm_->UnpinPage(0, false);
}


void minidb::Catalog::CreateTable(const std::string &table_name, const Schema &schema) {
    if (tables_.find(table_name) != tables_.end()) {
        throw std::runtime_error("Catalog Error: Table '" + table_name + "' already exists!");
    }
    auto table_heap = std::make_unique<TableHeap>(bpm_);
    tables_[table_name] = std::make_unique<TableMetaData>(schema, std::move(table_heap));
}

minidb::TableHeap * minidb::Catalog::GetTableHeap(const std::string &table_name) const {
    if (tables_.find(table_name) == tables_.end()) {
        throw std::runtime_error("Catalog Error: Table '" + table_name + "' not found!");
    }
    return tables_.at(table_name)->table_heap_.get();
}

const minidb::Schema & minidb::Catalog::GetSchema(const std::string &table_name) const {
    if (tables_.find(table_name) == tables_.end()) {
        throw std::runtime_error("Catalog Error: Table '" + table_name + "' not found!");
    }
    return tables_.at(table_name)->schema_;
}

void minidb::Catalog::save() const {
    Page *meta_data_page =meta_data_page=bpm_->FetchPage(0);
    assert(meta_data_page!=nullptr);
    SerializeTo(meta_data_page->GetData());
    bpm_->UnpinPage(0, true);
}

// +---------------------+ <--- 0 字节 (OFFSET_MAGIC_NUMBER)
// |   Magic Number (4字节)    |
// +---------------------+ <--- 4 字节 (OFFSET_TABLE_NUMBER)
// |   Table Number  (4字节) |
// +---------------------+ <--- 表的个数读取Table Number个表
// |    Table Name (string)
// +---------------------+
// |    First Page Id (4字节)
// +---------------------+
// |    Last Page Id (4字节)
// +---------------------+
// |    Column Count (4字节)
// +---------------------+ <--- 列个数
// |    Column Name(string)
// +---------------------+ <--- 列名
// |    Column Type Id(4字节)
// +---------------------+ <--- 列的类型
void minidb::Catalog::SerializeTo(char *dst) const {
    //写入魔数
    uint32_t magic_number=MAGIC_NUMBER;
    std::memcpy(dst+OFFSET_MAGIC_NUMBER,&magic_number,sizeof(magic_number));
    //写入表的数量
    uint32_t table_number=tables_.size();
    std::memcpy(dst+OFFSET_TABLE_NUMBER,&table_number,sizeof(table_number));
    //遍历表的名
    uint32_t offset=OFFSET_TABLE_INFO;
    for (const auto&[tb_name,tb_meta_data]:tables_ ) {
        assert(offset < PAGE_SIZE && "Catalog metadata exceeded page size!");
        //写入表名
        uint32_t name_len=tb_name.size();
        std::memcpy(dst+offset,&name_len,sizeof(name_len));
        offset+=sizeof(name_len);
        std::memcpy(dst+offset,tb_name.data(),tb_name.size());
        offset+=tb_name.size();
        //写入FirstPageId
        page_id_t first_page_id=tb_meta_data->table_heap_->GetFirstPageId();
        std::memcpy(dst+offset,&first_page_id,sizeof(page_id_t));
        offset+=sizeof(first_page_id);
        //写入LastPageId
        page_id_t last_page_id=tb_meta_data->table_heap_->GetLastPageId();
        std::memcpy(dst+offset,&last_page_id,sizeof(page_id_t));
        offset+=sizeof(last_page_id);
        //写入列元数据
        const  Schema&schema=tb_meta_data->schema_;
        uint32_t column_count=schema.GetColumnCount();
        std::memcpy(dst+offset,&column_count,sizeof(column_count));
        offset+=sizeof(column_count);
        for (const auto&column: schema.GetColumns()) {
            //写入列的名字
            uint32_t name_len=column.column_name_.size();
            std::memcpy(dst+offset,&name_len,sizeof(name_len));
            offset+=sizeof(name_len);
            std::memcpy(dst+offset,column.column_name_.c_str(),name_len);
            offset+=name_len;
            //写入列的类型
            std::memcpy(dst+offset,&column.column_type_,sizeof(column.column_type_));
            offset+=sizeof(column.column_type_);
        }

    }
}

void minidb::Catalog::DeserializeFrom(const char *src)  {
    //读入魔数
    uint32_t magic_number;
    std::memcpy(&magic_number,src+OFFSET_MAGIC_NUMBER,sizeof(magic_number));
    //读取表的数量
    uint32_t table_number=tables_.size();
    std::memcpy(&table_number,src+OFFSET_TABLE_NUMBER,sizeof(table_number));
    //遍历表
    uint32_t offset=OFFSET_TABLE_INFO;

    for (size_t i=0;i<table_number;i++) {

        std::string tb_name;
        uint32_t name_len;
        page_id_t first_page_id;
        page_id_t last_page_id;
        uint32_t column_count;
        std::vector<Column>columns;
        //读取表名
        std::memcpy(&name_len,src+offset,sizeof(name_len));
        tb_name.resize(name_len);
        offset+=sizeof(name_len);
        std::memcpy(tb_name.data(),src+offset,tb_name.size());
        offset+=tb_name.size();
        //读取FirstPageId
        std::memcpy(&first_page_id,src+offset,sizeof(page_id_t));
        offset+=sizeof(first_page_id);
        //读取LastPageId
        std::memcpy(&last_page_id,src+offset,sizeof(page_id_t));
        offset+=sizeof(last_page_id);
        //创建对应的tableHeap
        std::unique_ptr<TableHeap>tb_heap=std::make_unique<TableHeap>(bpm_,first_page_id,last_page_id);
        //读取列元数据
        std::memcpy(&column_count,src+offset,sizeof(column_count));
        offset+=sizeof(column_count);
        for (size_t i=0;i<column_count;i++) {
            //读取列的名字
            uint32_t name_len;
            std::memcpy(&name_len,src+offset,sizeof(name_len));
            offset+=sizeof(name_len);
            std::string column_name;column_name.resize(name_len);
            std::memcpy(column_name.data(),src+offset,name_len);
            offset+=name_len;
            //读取列的类型
            TypeId type_id;
            std::memcpy(&type_id,src+offset,sizeof(type_id));
            offset+=sizeof(type_id);
            columns.push_back(Column{column_name,type_id});
        }
        tables_[tb_name]=std::make_unique<TableMetaData>(Schema(columns),std::move(tb_heap));
    }
}

minidb::Column::Column(std::string name, TypeId type)
    :column_name_(name),column_type_(type)
{

}

minidb::Schema::Schema(const std::vector<Column> &columns)
:columns_(columns)
{
    for (const auto&col:columns) {
        type_ids_.push_back(col.column_type_);
    }
}

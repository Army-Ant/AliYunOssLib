#include "../inc/oss_requester.h"
#include <cstring>

#include "../inc/armyantlib/AADefine.h"
#include "../inc/armyantlib/AAClassPrivateHandle.hpp"
#include "../inc/aliyun-oss-c-sdk-3.5.0/aos_http_io.h"
#include "../inc/aliyun-oss-c-sdk-3.5.0/aos_util.h"
#include "../inc/aliyun-oss-c-sdk-3.5.0/oss_util.h"
#include "../inc/aliyun-oss-c-sdk-3.5.0/oss_api.h"

namespace ArmyAnt {

namespace AliYunTools {

namespace Oss {

class Requester_Private;
static ArmyAnt::ClassPrivateHandleManager<Requester, Requester_Private> sg_manager;

class Requester_Private {
public:
	Requester_Private();
	~Requester_Private();

public:
	inline bool checkInitOK()const;

public:
	bool has_initialized = false;
	aos_pool_t *pool = nullptr;
	oss_request_options_t *options = nullptr;

}; // class Requester_Private

Requester_Private::Requester_Private(){
	// 全局变量初始化
	auto init_ret = aos_http_io_initialize(NULL, 0);
	AAAssert(init_ret == AOSE_OK);
	if (init_ret != AOSE_OK) {
		return;
	}
	has_initialized = true;

	// 初始化内存池和options
	aos_pool_create(&pool, NULL);
	options = oss_request_options_create(pool);
	options->config = oss_config_create(options->pool);
	options->ctl = aos_http_controller_create(options->pool, 0);
}

Requester_Private::~Requester_Private() {
	// 释放内存pool资源，包括了通过pool分配的内存，比如options等
	if (pool != nullptr)
		aos_pool_destroy(pool);
	// 释放全局资源
	if (has_initialized)
		aos_http_io_deinitialize();
}

inline bool Requester_Private::checkInitOK()const {
	return has_initialized && pool != nullptr&&options != nullptr;
}


Requester::Requester(){
	sg_manager.GetHandle(this);
}

Requester::Requester(const char * endpoint, const char * access_key_id, const char * access_key_secret)
{
	sg_manager.GetHandle(this);
	setEndpoint(endpoint);
	setAccessKeyId(access_key_id);
	setAccessKeySecret(access_key_secret);
	setIsCName(false);
}

Requester::~Requester()
{
	sg_manager.ReleaseHandle(this);
}

void Requester::setEndpoint(const char * endpoint)
{
	aos_str_set(&sg_manager[this]->options->config->endpoint, endpoint);
}

void Requester::setAccessKeyId(const char * access_key_id)
{
	aos_str_set(&sg_manager[this]->options->config->access_key_id, access_key_id);
}

void Requester::setAccessKeySecret(const char * access_key_secret)
{
	aos_str_set(&sg_manager[this]->options->config->access_key_secret, access_key_secret);
}

void Requester::setIsCName(bool is_c_name)
{
	sg_manager[this]->options->config->is_cname = (is_c_name ? 1 : 0);
}

bool Requester::getEndpoint(char * endpoint) const
{
	auto ret = sg_manager[this]->options->config->endpoint;
	if (endpoint == nullptr || ret.data == nullptr)
		return false;
	strcpy(endpoint, ret.data);
	return true;
}

bool Requester::getAccessKeyId(char * access_key_id) const
{
	auto ret = sg_manager[this]->options->config->access_key_id;
	if (access_key_id == nullptr || ret.data == nullptr)
		return false;
	strcpy(access_key_id, ret.data);
	return true;
}

bool Requester::getAccessKeySecret(char * access_key_secret) const
{
	auto ret = sg_manager[this]->options->config->access_key_secret;
	if (access_key_secret == nullptr || ret.data == nullptr)
		return false;
	strcpy(access_key_secret, ret.data);
	return true;
}

bool Requester::getIsCName() const
{
	return sg_manager[this]->options->config->is_cname != 0;
}

bool Requester::createBucket(const char * bucket_name)
{
	if (!sg_manager[this]->checkInitOK())
		return false;

	aos_string_t bucket;
	/* 初始化参数 */
	aos_str_set(&bucket, bucket_name);
	oss_acl_e oss_acl = OSS_ACL_PRIVATE;
	aos_table_t *resp_headers;
	auto s = oss_create_bucket(sg_manager[this]->options, &bucket, oss_acl, &resp_headers);
	return aos_status_is_ok(s);
}

bool Requester::deleteBucket(const char * bucket_name)
{
	if (!sg_manager[this]->checkInitOK())
		return false;

	aos_string_t bucket;
	/* 初始化参数 */
	aos_str_set(&bucket, bucket_name);
	aos_table_t *resp_headers;
	auto s = oss_delete_bucket(sg_manager[this]->options, &bucket, &resp_headers);
	return aos_status_is_ok(s);
}

bool Requester::getBucketAcl(const char * bucket_name)
{
	if (!sg_manager[this]->checkInitOK())
		return false;

	aos_string_t bucket;
	/* 初始化参数 */
	aos_str_set(&bucket, bucket_name);
	aos_table_t *resp_headers;
	aos_string_t oss_acl;
	auto s = oss_get_bucket_acl(sg_manager[this]->options, &bucket, &oss_acl, &resp_headers);
	return aos_status_is_ok(s);
}

bool Requester::setBucketAcl(const char * bucket_name, BucketAcl acl)
{
	if (!sg_manager[this]->checkInitOK())
		return false;

	aos_string_t bucket;
	/* 初始化参数 */
	aos_str_set(&bucket, bucket_name);

	aos_table_t *resp_headers;
	oss_acl_e oss_acl = OSS_ACL_PRIVATE;
	auto s = oss_put_bucket_acl(sg_manager[this]->options, &bucket, oss_acl, &resp_headers);
	return aos_status_is_ok(s);
}

bool Requester::uploadData(const char * bucket_name, const char * object_name, const void * data, int length)
{
	if (!sg_manager[this]->checkInitOK())
		return false;

	aos_string_t bucket;
	aos_string_t object;
	aos_list_t buffer;
	/* 初始化参数 */
	aos_str_set(&bucket, bucket_name);
	aos_str_set(&object, object_name);
	aos_list_init(&buffer);
	aos_buf_t*content = aos_buf_pack(sg_manager[this]->options->pool, data, length);
	aos_list_add_tail(&content->node, &buffer);


	/* 上传文件 */
	aos_table_t *headers = NULL;
	aos_table_t *resp_headers = NULL;
	auto s = oss_put_object_from_buffer(sg_manager[this]->options, &bucket, &object,&buffer, headers, &resp_headers);

	return aos_status_is_ok(s);
}

bool Requester::uploadAppendData(const char * bucket_name, const char * object_name, const void * data, int length)
{
	if (!sg_manager[this]->checkInitOK())
		return false;

	aos_string_t bucket;
	aos_string_t object;
	aos_list_t buffer;
	aos_table_t *resp_headers = NULL;
	/* 初始化参数 */
	aos_str_set(&bucket, bucket_name);
	aos_str_set(&object, object_name);
	aos_list_init(&buffer);
	aos_buf_t*content = aos_buf_pack(sg_manager[this]->options->pool, data, length);
	aos_list_add_tail(&content->node, &buffer);

	/* 获取起始追加位置 */
	char *next_append_position = NULL;
	int64 position = 0;
	auto s = oss_head_object(sg_manager[this]->options, &bucket, &object, aos_table_make(sg_manager[this]->pool, 0), &resp_headers);
	if (aos_status_is_ok(s)) {
		next_append_position = (char*)(apr_table_get(resp_headers, "x-oss-next-append-position"));
		position = atoi(next_append_position);
	}
	else {
		return false;
	}

	/* 追加文件 */
	aos_list_init(&buffer);
	content = aos_buf_pack(sg_manager[this]->pool, data, length);
	aos_list_add_tail(&content->node, &buffer);
	s = oss_append_object_from_buffer(sg_manager[this]->options, &bucket, &object, position, &buffer, aos_table_make(sg_manager[this]->pool, 0), &resp_headers);

	return aos_status_is_ok(s);
}

bool Requester::uploadFile(const char * bucket_name, const char * object_name, const char * path)
{
	if (!sg_manager[this]->checkInitOK())
		return false;

	aos_string_t bucket;
	aos_string_t object;
	/* 初始化参数 */
	aos_table_t *resp_headers = NULL;
	auto headers = aos_table_make(sg_manager[this]->options->pool, 1);
	aos_str_set(&bucket, bucket_name);
	aos_str_set(&object, object_name);    
	
	/* 上传文件 */
	aos_string_t file;
	aos_str_set(&file, path);
	auto s = oss_put_object_from_file(sg_manager[this]->options, &bucket, &object, &file, headers, &resp_headers);

	return aos_status_is_ok(s);
}

bool Requester::uploadAppendFile(const char * bucket_name, const char * object_name, const char * path)
{
	if (!sg_manager[this]->checkInitOK())
		return false;

	aos_string_t bucket;
	aos_string_t object;
	aos_table_t *resp_headers = NULL;
	/* 初始化参数 */
	aos_str_set(&bucket, bucket_name);
	aos_str_set(&object, object_name);
	aos_string_t file;
	aos_str_set(&file, path);

	/* 获取起始追加位置 */
	char *next_append_position = NULL;
	int64 position = 0;
	auto s = oss_head_object(sg_manager[this]->options, &bucket, &object, aos_table_make(sg_manager[this]->pool, 0), &resp_headers);
	if (aos_status_is_ok(s)) {
		next_append_position = (char*)(apr_table_get(resp_headers, "x-oss-next-append-position"));
		position = atoi(next_append_position);
	}
	else {
		return false;
	}

	/* 追加文件 */
    s = oss_append_object_from_file(sg_manager[this]->options, &bucket, &object, position, &file, aos_table_make(sg_manager[this]->pool, 0), &resp_headers);

	return aos_status_is_ok(s);
}

bool Requester::uploadFileWithPart(const char * bucket_name, const char * object_name, const char * path, int partSize, int threadNum)
{
	if (!sg_manager[this]->checkInitOK())
		return false;

	aos_string_t bucket;
	aos_string_t object;
	/* 初始化参数 */
	aos_table_t *resp_headers = NULL;
	auto headers = aos_table_make(sg_manager[this]->options->pool, 1);
	aos_string_t upload_id;
	aos_str_null(&upload_id);
	aos_str_set(&bucket, bucket_name);
	aos_str_set(&object, object_name);

	/* 分片上传 */
	aos_string_t file;
	aos_str_set(&file, path);  
	auto s = oss_upload_file(sg_manager[this]->options, &bucket, &object, &upload_id, &file, partSize, nullptr);

	return aos_status_is_ok(s);
}

} // namespace Oss

} // namespace AliYunTools

} // namespace ArmyAnt
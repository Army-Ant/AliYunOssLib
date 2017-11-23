#ifndef OSS_REQUESTER_H_2017_11_12
#define OSS_REQUESTER_H_2017_11_12

namespace ArmyAnt {

namespace AliYunTools {

namespace Oss {

enum class BucketAcl : char {
	Private,
	Read,
	Write
};

class Requester {
public:
	Requester();
	Requester(const char*endpoint, const char*access_key_id, const char*access_key_secret);
	~Requester();

public: // request settings
	void setEndpoint(const char*endpoint);
	void setAccessKeyId(const char*access_key_id);
	void setAccessKeySecret(const char*access_key_secret);
	void setIsCName(bool is_c_name);

	void setStsToken(const char*sts_token);

	bool getEndpoint(char*endpoint)const;
	bool getAccessKeyId(char*access_key_id)const;
	bool getAccessKeySecret(char*access_key_secret)const;
	bool getIsCName()const;

public: // bucket management
	bool createBucket(const char*bucket_name);
	bool deleteBucket(const char*bucket_name);
	bool getBucketAcl(const char*bucket_name);
	bool setBucketAcl(const char*bucket_name, BucketAcl acl);

public: // file upload
	bool uploadData(const char*bucket_name, const char*object_name, const void*data, int length);
	// Append data to an existing object, it will append to the tail, if the object does not exist, it will be create. 
	bool uploadAppendData(const char*bucket_name, const char*object_name, const void*data, int length);
	bool uploadFile(const char*bucket_name, const char*object_name, const char*path);
	bool uploadAppendFile(const char*bucket_name, const char*object_name, const char*path);
	//分片上传
	//TODO: 需要设计分步的分片上传, 以及断点续传
	bool uploadFileWithPart(const char*bucket_name, const char*object_name, const char*path, int partSize, int threadNum = 1);
	//TODO: 完成文件元信息的设置方法
	bool setFileInfo(/*...*/);

public: // file download
	bool downloadToBuffer(const char*bucket_name, const char*object_name, void*buffer, long long rangeStart = 0, long long rangeEnd = 0);
	bool downloadToFile(const char*bucket_name, const char*object_name, const char*path, long long rangeStart = 0, long long rangeEnd = 0);
	bool downloadFileByPart(const char*bucket_name, const char*object_name, const char*path, long long partSize, int threadNum = 1);
	
public: // file management
	bool listFile(const char*bucket_name);
	bool getFileInfo(/*...*/);
	bool deleteFile(const char*bucket_name, const char*object_name);
	bool deleteFilesWithPrefix(const char*bucket_name, const char*prefix);

public: // gen access
	bool createDownloadUrlAccess(const char*bucket_name, const char*object_name);
	bool createUploadUrlAccess(const char*bucket_name, const char*object_name);
	bool createImageUrlAccess(const char*bucket_name, const char*object_name);

public: // life cycle

public: // image operator
	
}; // class RequestOptions

} // namespace Oss

} // namespace AliYunTools

} // namespace ArmyAnt

#endif // OSS_REQUESTER_H_2017_11_12
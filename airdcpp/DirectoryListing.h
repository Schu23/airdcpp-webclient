/*
 * Copyright (C) 2001-2015 Jacek Sieka, arnetheduck on gmail point com
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifndef DCPLUSPLUS_DCPP_DIRECTORY_LISTING_H
#define DCPLUSPLUS_DCPP_DIRECTORY_LISTING_H

#include "forward.h"

#include "DirectoryListingListener.h"
#include "ClientManagerListener.h"
#include "DownloadManagerListener.h"
#include "SearchManagerListener.h"
#include "ShareManagerListener.h"
#include "TimerManager.h"

#include "AirUtil.h"
#include "Bundle.h"
#include "FastAlloc.h"
#include "GetSet.h"
#include "HintedUser.h"
#include "MerkleTree.h"
#include "Pointer.h"
#include "QueueItemBase.h"
#include "TaskQueue.h"
#include "UserInfoBase.h"
#include "SearchResult.h"
#include "ShareManager.h"
#include "Streams.h"
#include "TargetUtil.h"

namespace dcpp {

class ListLoader;
typedef uint32_t DirectoryListingToken;

class DirectoryListing : public intrusive_ptr_base<DirectoryListing>, public UserInfoBase, 
	public Speaker<DirectoryListingListener>, private SearchManagerListener, private TimerManagerListener, 
	private ClientManagerListener, private ShareManagerListener, private DownloadManagerListener
{
public:
	class Directory;
	class File {

	public:
		typedef File* Ptr;
		struct Sort { bool operator()(const Ptr& a, const Ptr& b) const; };

		typedef std::vector<Ptr> List;
		typedef List::const_iterator Iter;
		
		File(Directory* aDir, const string& aName, int64_t aSize, const TTHValue& aTTH, bool checkDupe, time_t aRemoteDate) noexcept;
		File(const File& rhs, bool _adls = false) noexcept;

		~File() { }


		string getPath() const noexcept {
			return parent->getPath() + name;
		}

		GETSET(string, name, Name);
		GETSET(int64_t, size, Size);
		GETSET(Directory*, parent, Parent);
		GETSET(TTHValue, tthRoot, TTH);
		IGETSET(bool, adls, Adls, false);
		IGETSET(DupeType, dupe, Dupe, DUPE_NONE);
		IGETSET(time_t, remoteDate, RemoteDate, 0);
		bool isQueued() const noexcept {
			return (dupe == DUPE_QUEUE || dupe == DUPE_FINISHED);
		}

		DirectoryListingToken getToken() const noexcept {
			return token;
		}
	private:
		const DirectoryListingToken token;
	};

	class Directory : boost::noncopyable, public intrusive_ptr_base<Directory> {
	public:
		enum DirType {
			TYPE_NORMAL,
			TYPE_INCOMPLETE_CHILD,
			TYPE_INCOMPLETE_NOCHILD,
			TYPE_ADLS,
		};

		//typedef Directory* Ptr;
		typedef boost::intrusive_ptr<Directory> Ptr;

		struct Sort { bool operator()(const Ptr& a, const Ptr& b) const; };

		typedef std::vector<Ptr> List;
		typedef List::const_iterator Iter;
		typedef unordered_set<TTHValue> TTHSet;
		
		List directories;
		File::List files;

		Directory(Directory* aParent, const string& aName, DirType aType, time_t aUpdateDate, bool checkDupe = false, const string& aSize = Util::emptyString, time_t aRemoteDate = 0);

		virtual ~Directory();

		size_t getTotalFileCount(bool countAdls) const noexcept;
		int64_t getTotalSize(bool countAdls) const noexcept;
		void filterList(DirectoryListing& dirList) noexcept;
		void filterList(TTHSet& l) noexcept;
		void getHashList(TTHSet& l) const noexcept;
		void clearAdls() noexcept;
		void clearAll() noexcept;

		bool findIncomplete() const noexcept;
		void search(OrderedStringSet& aResults, SearchQuery& aStrings) const noexcept;
		void findFiles(const boost::regex& aReg, File::List& aResults) const noexcept;
		
		size_t getFileCount() const noexcept { return files.size(); }
		size_t getFolderCount() const noexcept { return directories.size(); }
		
		int64_t getFilesSize() const noexcept;

		string getPath() const noexcept;
		uint8_t checkShareDupes() noexcept;
		
		GETSET(string, name, Name);
		IGETSET(int64_t, partialSize, PartialSize, 0);
		GETSET(Directory*, parent, Parent);
		GETSET(DirType, type, Type);
		IGETSET(DupeType, dupe, Dupe, DUPE_NONE);
		IGETSET(time_t, remoteDate, RemoteDate, 0);
		IGETSET(time_t, updateDate, UpdateDate, 0);
		IGETSET(bool, loading, Loading, false);

		bool isComplete() const noexcept { return type == TYPE_ADLS || type == TYPE_NORMAL; }
		void setComplete() noexcept { type = TYPE_NORMAL; }
		bool getAdls() const noexcept { return type == TYPE_ADLS; }

		void download(const string& aTarget, BundleFileInfo::List& aFiles) noexcept;

		DirectoryListingToken getToken() const noexcept {
			return token;
		}
	private:
		const DirectoryListingToken token;
	};

	class AdlDirectory : public Directory {
	public:
		AdlDirectory(const string& aFullPath, Directory* aParent, const string& aName) : Directory(aParent, aName, Directory::TYPE_ADLS, GET_TIME()), fullPath(aFullPath) { }

		GETSET(string, fullPath, FullPath);
	};

	DirectoryListing(const HintedUser& aUser, bool aPartial, const string& aFileName, bool isClientView, const string& aDirectory, bool aIsOwnList=false);
	~DirectoryListing();
	
	void loadFile() throw(Exception, AbortException);

	//return the number of loaded dirs
	int updateXML(const string& aXml, const string& aBase) throw(AbortException);

	//return the number of loaded dirs
	int loadXML(InputStream& xml, bool updating, const string& aBase = "/", time_t aListDate = GET_TIME()) throw(AbortException);

	bool downloadDir(const string& aRemoteDir, const string& aTarget, QueueItemBase::Priority prio = QueueItem::DEFAULT, ProfileToken aAutoSearch = 0) noexcept;
	bool createBundle(Directory::Ptr& aDir, const string& aTarget, QueueItemBase::Priority prio, ProfileToken aAutoSearch) noexcept;

	void openFile(const File* aFile, bool isClientView) const throw(QueueException, FileException);

	int64_t getTotalListSize(bool adls = false) const noexcept { return root->getTotalSize(adls); }
	int64_t getDirSize(const string& aDir) const noexcept;
	size_t getTotalFileCount(bool adls = false) const noexcept { return root->getTotalFileCount(adls); }

	const Directory::Ptr getRoot() const noexcept { return root; }
	Directory::Ptr getRoot() noexcept { return root; }
	void getLocalPaths(const Directory::Ptr& d, StringList& ret) const throw(ShareException);
	void getLocalPaths(const File* f, StringList& ret) const throw(ShareException);

	bool isMyCID() const noexcept;
	string getNick(bool firstOnly) const noexcept;
	static string getNickFromFilename(const string& fileName) noexcept;
	static UserPtr getUserFromFilename(const string& fileName) noexcept;
	
	const UserPtr& getUser() const noexcept { return hintedUser.user; }
	const HintedUser& getHintedUser() const noexcept { return hintedUser; }
	const string& getHubUrl() const noexcept { return hintedUser.hint; }
	void setHubUrl(const string& newUrl, bool isGuiChange) noexcept;
		
	GETSET(bool, partialList, PartialList);
	GETSET(bool, isOwnList, IsOwnList);
	GETSET(bool, isClientView, isClientView);
	GETSET(string, fileName, FileName);
	GETSET(bool, matchADL, MatchADL);
	IGETSET(bool, closing, Closing, false);
	IGETSET(optional<QueueToken>, queueToken, QueueToken, boost::none);

	typedef std::function<void(const string& aPath)> DupeOpenF;
	void addViewNfoTask(const string& aDir, bool aAllowQueueList, DupeOpenF aDupeF = nullptr) noexcept;
	void addMatchADLTask() noexcept;
	void addListDiffTask(const string& aFile, bool aOwnList) noexcept;
	void addPartialListTask(const string& aXml, const string& aBase, bool reloadAll = false, bool changeDir = true, std::function<void()> f = nullptr) noexcept;
	void addFullListTask(const string& aDir) noexcept;
	void addQueueMatchTask() noexcept;

	void addAsyncTask(DispatcherQueue::Callback&& f) noexcept;
	void close() noexcept;

	void addSearchTask(const string& aSearchString, int64_t aSize, int aTypeMode, int aSizeMode, const StringList& aExtList, const string& aDir) noexcept;
	bool nextResult(bool prev) noexcept;

	unique_ptr<SearchQuery> curSearch = nullptr;

	bool isCurrentSearchPath(const string& path) const noexcept;
	size_t getResultCount() const noexcept { return searchResults.size(); }

	Directory::Ptr findDirectory(const string& aName) const noexcept { return findDirectory(aName, root); }
	Directory::Ptr findDirectory(const string& aName, const Directory::Ptr& current) const noexcept;
	
	bool supportsASCH() const noexcept;

	void onRemovedQueue(const string& aDir) noexcept;

	/* only call from the file list thread*/
	bool downloadDirImpl(Directory::Ptr& aDir, const string& aTarget, QueueItemBase::Priority prio, ProfileToken aAutoSearch) noexcept;
	void setActive() noexcept;

	enum State : uint8_t {
		STATE_DOWNLOAD_PENDING,
		STATE_DOWNLOADING,
		STATE_LOADING,
		STATE_LOADED
	};

	State getState() const noexcept {
		return state;
	}

	bool isOpen() const noexcept {
		return open;
	}

	enum ReloadMode {
		RELOAD_NONE,
		RELOAD_DIR,
		RELOAD_ALL
	};

	// Returns false if the directory was not found from the list
	bool changeDirectory(const string& aPath, ReloadMode aReloadMode, bool aIsSearchChange = false) noexcept;

	struct LocationInfo {
		string path;

		int64_t size = -1;
		int files = -1;
		int directories = -1;
		bool complete = true;
	};

	const LocationInfo& getCurrentLocationInfo() const noexcept {
		return currentLocation;
	}
private:
	LocationInfo currentLocation;
	void updateCurrentLocation(const Directory::Ptr& aCurrentDirectory) noexcept;

	friend class ListLoader;

	Directory::Ptr root;

	//maps loaded base dirs with their full lowercase paths and whether they've been visited or not
	typedef unordered_map<string, pair<Directory::Ptr, bool>> DirMap;
	DirMap baseDirs;

	void dispatch(DispatcherQueue::Callback& aCallback) noexcept;

	bool open = false;
	State state = STATE_DOWNLOAD_PENDING;
	void setState(State aState) noexcept;

	atomic_flag running;

	void on(SearchManagerListener::SR, const SearchResultPtr& aSR) noexcept;

	// ClientManagerListener
	void on(ClientManagerListener::DirectSearchEnd, const string& aToken, int resultCount) noexcept;
	void on(ClientManagerListener::UserDisconnected, const UserPtr& aUser, bool wentOffline) noexcept;
	void on(ClientManagerListener::UserUpdated, const OnlineUser& aUser) noexcept;

	void on(TimerManagerListener::Second, uint64_t aTick) noexcept;

	// ShareManagerListener
	void on(ShareManagerListener::DirectoriesRefreshed, uint8_t, const RefreshPathList& aPaths) noexcept;

	void on(DownloadManagerListener::Failed, const Download* aDownload, const string& aReason) noexcept;
	void on(DownloadManagerListener::Starting, const Download* aDownload) noexcept;

	void endSearch(bool timedOut = false) noexcept;

	int loadShareDirectory(const string& aPath, bool aRecurse = false) throw(Exception, AbortException);

	OrderedStringSet searchResults;
	OrderedStringSet::iterator curResult;

	int curResultCount = 0;
	int maxResultCount = 0;
	uint64_t lastResult = 0;
	string searchToken;

	void listDiffImpl(const string& aFile, bool aOwnList) throw(Exception, AbortException);
	void loadFileImpl(const string& aInitialDir) throw(Exception, AbortException);
	void searchImpl(const string& aSearchString, int64_t aSize, int aTypeMode, int aSizeMode, const StringList& aExtList, const string& aDir) noexcept;
	void loadPartialImpl(const string& aXml, const string& aBaseDir, bool reloadAll, bool changeDir, std::function<void()> completionF) throw(Exception, AbortException);
	void matchAdlImpl() throw(AbortException);
	void matchQueueImpl() noexcept;
	void removedQueueImpl(const string& aDir) noexcept;
	void findNfoImpl(const string& aPath, bool aAllowQueueList, DupeOpenF aDupeF) noexcept;

	HintedUser hintedUser;

	void checkShareDupes() noexcept;
	void onLoadingFinished(int64_t aStartTime, const string& aDir, bool aReloadList, bool aChangeDir) noexcept;

	void statusMessage(const string& aText, LogMessage::Severity aSeverity) noexcept;

	DispatcherQueue tasks;
};

inline bool operator==(const DirectoryListing::Directory::Ptr& a, const string& b) { return Util::stricmp(a->getName(), b) == 0; }
inline bool operator==(const DirectoryListing::File::Ptr& a, const string& b) { return Util::stricmp(a->getName(), b) == 0; }

} // namespace dcpp

#endif // !defined(DIRECTORY_LISTING_H)

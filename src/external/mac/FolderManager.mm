#include "FolderManager.h"
#import <Foundation/Foundation.h>

using namespace fm;

FolderManager::FolderManager() {
  m_autoreleasePool = [[NSAutoreleasePool alloc] init];
}

FolderManager::~FolderManager() {
  [(NSAutoreleasePool *) m_autoreleasePool release];
}

const char *FolderManager::pathForDirectory(SearchPathDirectory directory, SearchPathDomainMask domainMask) {
  NSFileManager *fileManager = [NSFileManager defaultManager];
  NSArray *URLs = [fileManager URLsForDirectory:(NSSearchPathDirectory) directory inDomains:static_cast<NSSearchPathDomainMask>(domainMask)];
  if (URLs.count == 0) return NULL;

  NSURL *URL = [URLs objectAtIndex:0];
  NSString *path = URL.path;

  // `fileSystemRepresentation` on an `NSString` gives a path suitable for POSIX APIs
  return path.fileSystemRepresentation;
}

const char *FolderManager::pathForDirectoryAppropriateForItemAtPath(SearchPathDirectory directory,
                                                                    SearchPathDomainMask domainMask,
                                                                    const char *itemPath, bool create) {

  NSFileManager *fileManager = [NSFileManager defaultManager];
  NSString *nsPath = [fileManager stringWithFileSystemRepresentation:itemPath length:strlen(itemPath)];
  NSURL *itemURL = (nsPath ? [NSURL fileURLWithPath:nsPath] : nil);

  NSURL *URL = [fileManager URLForDirectory:(NSSearchPathDirectory) directory
                                   inDomain:static_cast<NSSearchPathDomainMask>(domainMask)
                          appropriateForURL:itemURL
                                     create:create error:NULL];
  return URL.path.fileSystemRepresentation;
}

const char *FolderManager::pathForResource(const char *resource) {
  CFStringRef file = CFStringCreateWithCString(nullptr, resource, kCFStringEncodingUTF8);
  CFURLRef appUrlRef = CFBundleCopyResourceURL(CFBundleGetMainBundle(), file, nullptr, nullptr);
  CFStringRef filePathRef = CFURLCopyPath(appUrlRef);

  return CFStringGetCStringPtr(filePathRef, kCFStringEncodingUTF8);
}

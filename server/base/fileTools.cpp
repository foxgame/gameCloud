#include "fileTools.h"

#include <deque>
#include <utility>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>

bool fileTools::getSubFiles( const string& path , fileVector& files , int max_depth , bool include_sub_dirs , traverse_order_t order )
    {
        using namespace std;
        namespace fs = boost::filesystem;
        typedef std::pair<string, int> path_and_depth_t;
        deque<path_and_depth_t> qu;
        {
            fs::path root(path);
            if (!fs::exists(root) || !fs::is_directory(root))
            {
                return false;
            }
            if (max_depth <= 0 && max_depth != UNLIMITED_DEPTH)
            {
                return true;
            }
            fs::directory_iterator end_iter;
            for (fs::directory_iterator file_itr(root); file_itr != end_iter; ++file_itr)
            {
                qu.push_back(path_and_depth_t(fs::system_complete(*file_itr).string(), 1));
            }
        }
        while (!qu.empty())
        {
            path_and_depth_t path_and_depth = (order == DEPTH_FIRST) ? qu.back() : qu.front();
            string& file_str(path_and_depth.first);
            int depth = path_and_depth.second;
            if (order == DEPTH_FIRST)
            {
                qu.pop_back();
            }
            else
            {
                qu.pop_front();
            }
            fs::path file(file_str);
            if (fs::exists(file))
            {
                if (fs::is_directory(file))
                {
                    if (include_sub_dirs)
                    {
                        files.push_back(file_str);
                    }
                    if (depth < max_depth || max_depth == UNLIMITED_DEPTH)
                    {
                        int next_depth = depth + 1;
                        fs::directory_iterator end_iter;
                        for (fs::directory_iterator file_itr(file); file_itr != end_iter; ++file_itr)
                        {
                            qu.push_back(path_and_depth_t(fs::system_complete(*file_itr).string(), next_depth));
                        }
                    }
                }
                else
                {
                    files.push_back(file_str);
                }
            }
        }
        return true;
    }

/*
 * libretroshare/src/services p3gxscommon.cc
 *
 * GxsChannels interface for RetroShare.
 *
 * Copyright 2012-2013 by Robert Fernie.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License Version 2.1 as published by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 * USA.
 *
 * Please report all bugs and problems to "retroshare@lunamutt.com".
 *
 */

#include "retroshare/rsgxscommon.h"
#include "services/p3gxscommon.h"
#include "serialiser/rsgxscommentitems.h"

#include <stdio.h>


RsGxsComment::RsGxsComment()
{
	mUpVotes = 0;
	mDownVotes = 0;
	score = 0;
}

RsGxsImage::RsGxsImage()
{
	mData = NULL;
	mSize = 0;
}


RsGxsFile::RsGxsFile()
{
	mSize = 0;
}

RsGxsVote::RsGxsVote()
{
	mVoteType = 0;
}


/********************************************************************************/
/******************* Startup / Tick    ******************************************/
/********************************************************************************/

p3GxsCommentService::p3GxsCommentService(RsGenExchange *exchange, uint16_t service_type)
    : mExchange(exchange), mServiceType(service_type)
{
	return;
}

bool p3GxsCommentService::getGxsCommentData(const uint32_t &token, std::vector<RsGxsComment> &comments)
{
	std::cerr << "p3GxsCommentService::getGxsCommentData()";
	std::cerr << std::endl;

	GxsMsgDataMap msgData;
	bool ok = mExchange->getMsgData(token, msgData);
		
	if(ok)
	{
		GxsMsgDataMap::iterator mit = msgData.begin();
		std::multimap<RsGxsMessageId, RsGxsVoteItem *> voteMap;
		
		for(; mit != msgData.end();  mit++)
		{
			RsGxsGroupId grpId = mit->first;
			std::vector<RsGxsMsgItem*>& msgItems = mit->second;
			std::vector<RsGxsMsgItem*>::iterator vit = msgItems.begin();

			/* now split into Comments and Votes */
		
			for(; vit != msgItems.end(); vit++)
			{
				RsGxsCommentItem* item = dynamic_cast<RsGxsCommentItem*>(*vit);
		
				if(item)
				{
					RsGxsComment comment = item->mMsg;
					comment.mMeta = item->meta;
					comments.push_back(comment);
					delete item;
				}
				else
				{
					RsGxsVoteItem* vote = dynamic_cast<RsGxsVoteItem*>(*vit);
					if (vote)
					{
						voteMap.insert(std::make_pair(vote->meta.mParentId, vote));
					}
					else
					{
						std::cerr << "Not a Comment or Vote, deleting!" << std::endl;
						delete *vit;
					}
				}
			}
		}

		/* now iterate through comments - and set the vote counts */
		std::vector<RsGxsComment>::iterator cit;
		std::multimap<RsGxsMessageId, RsGxsVoteItem *>::iterator it;
		for(cit = comments.begin(); cit != comments.end(); cit++)
		{
			for (it = voteMap.lower_bound(cit->mMeta.mMsgId); it != voteMap.upper_bound(cit->mMeta.mMsgId); it++)
			{
				if (it->second->mMsg.mVoteType == GXS_VOTE_UP)
				{
					cit->mUpVotes++;
				}
				else
				{
					cit->mDownVotes++;
				}
			}
		}

		std::cerr << "p3GxsCommentService::getGxsCommentData() Found " << comments.size() << " Comments";
		std::cerr << std::endl;
		std::cerr << "p3GxsCommentService::getGxsCommentData() Found " << voteMap.size() << " Votes";
		std::cerr << std::endl;

		/* delete the votes */
		for (it = voteMap.begin(); it != voteMap.end(); it++)
		{
			delete it->second;
		}


		
	}
		
	return ok;
}


bool p3GxsCommentService::getGxsRelatedComments(const uint32_t &token, std::vector<RsGxsComment> &comments)
{
	std::cerr << "p3GxsCommentService::getGxsRelatedComments()";
	std::cerr << std::endl;

	GxsMsgRelatedDataMap msgData;
	bool ok = mExchange->getMsgRelatedData(token, msgData);
			
	if(ok)
	{
		GxsMsgRelatedDataMap::iterator mit = msgData.begin();
		std::multimap<RsGxsMessageId, RsGxsVoteItem *> voteMap;
		
		for(; mit != msgData.end();  mit++)
		{
			std::vector<RsGxsMsgItem*>& msgItems = mit->second;
			std::vector<RsGxsMsgItem*>::iterator vit = msgItems.begin();
			
			for(; vit != msgItems.end(); vit++)
			{
				RsGxsCommentItem* item = dynamic_cast<RsGxsCommentItem*>(*vit);
		
				if(item)
				{
					RsGxsComment comment = item->mMsg;
					comment.mMeta = item->meta;
					comments.push_back(comment);
					delete item;
				}
				else
				{
					RsGxsVoteItem* vote = dynamic_cast<RsGxsVoteItem*>(*vit);
					if (vote)
					{
						voteMap.insert(std::make_pair(vote->meta.mParentId, vote));
					}
					else
					{
						std::cerr << "Not a Comment or Vote, deleting!" << std::endl;
						delete *vit;
					}
				}
			}
		}

		/* now iterate through comments - and set the vote counts */
		std::vector<RsGxsComment>::iterator cit;
		std::multimap<RsGxsMessageId, RsGxsVoteItem *>::iterator it;
		for(cit = comments.begin(); cit != comments.end(); cit++)
		{
			for (it = voteMap.lower_bound(cit->mMeta.mMsgId); it != voteMap.upper_bound(cit->mMeta.mMsgId); it++)
			{
				if (it->second->mMsg.mVoteType == GXS_VOTE_UP)
				{
					cit->mUpVotes++;
				}
				else
				{
					cit->mDownVotes++;
				}
			}
		}

		std::cerr << "p3GxsCommentService::getGxsRelatedComments() Found " << comments.size() << " Comments";
		std::cerr << std::endl;
		std::cerr << "p3GxsCommentService::getGxsRelatedComments() Found " << voteMap.size() << " Votes";
		std::cerr << std::endl;

		/* delete the votes */
		for (it = voteMap.begin(); it != voteMap.end(); it++)
		{
			delete it->second;
		}
	}
			
	return ok;
}

/********************************************************************************************/

bool p3GxsCommentService::createGxsComment(uint32_t &token, RsGxsComment &msg)
{
	std::cerr << "p3GxsChannels::createGxsComment() GroupId: " << msg.mMeta.mGroupId;
	std::cerr << std::endl;

	RsGxsCommentItem* msgItem = new RsGxsCommentItem(mServiceType);
	msgItem->mMsg = msg;
	msgItem->meta = msg.mMeta;
	
	mExchange->publishMsg(token, msgItem);
	return true;
}


bool p3GxsCommentService::createGxsVote(uint32_t &token, RsGxsVote &msg)
{
	std::cerr << "p3GxsChannels::createGxsVote() GroupId: " << msg.mMeta.mGroupId;
	std::cerr << std::endl;

	RsGxsVoteItem* msgItem = new RsGxsVoteItem(mServiceType);
	msgItem->mMsg = msg;
	msgItem->meta = msg.mMeta;
	
	mExchange->publishMsg(token, msgItem);
	return true;
}


/********************************************************************************************/
/********************************************************************************************/

#if 0
void p3GxsCommentService::setMessageReadStatus(uint32_t& token, const RsGxsGrpMsgIdPair& msgId, bool read)
{
	uint32_t mask = GXS_SERV::GXS_MSG_STATUS_UNREAD | GXS_SERV::GXS_MSG_STATUS_UNPROCESSED;
	uint32_t status = GXS_SERV::GXS_MSG_STATUS_UNREAD;
	if (read)
	{
		status = 0;
	}

	setMsgStatusFlags(token, msgId, status, mask);

}

#endif

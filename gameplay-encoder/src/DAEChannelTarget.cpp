
#include "DAEChannelTarget.h"


namespace gameplay
{

DAEChannelTarget::DAEChannelTarget(const domChannel* channelRef) : channel(NULL), targetElement(NULL)
{
    channel = channelRef;
    const std::string target = channelRef->getTarget();
    size_t index = target.find('/');
    if (index == std::string::npos)
    {
        // If the string doesn't contain a '/' then the whole string is the id
        // and there are no sid's being targeted.
        targetId = target;
    }
    else
    {
        // The targetId is the part before the first '/'
        targetId = target.substr(0, index);

        // each '/' denotes another sid
        size_t start;
        size_t end;
        do
        {
            start = index + 1;
            end = target.find('/', start);
        
            std::string sub;
            if (end == std::string::npos)
            {
                sub = target.substr(start);
                // break;
            }
            else
            {
                sub = target.substr(start, end - start);
                index = end + 1;
            }
            attributeIds.push_back(sub);
        } while(end != std::string::npos);
    }

}

DAEChannelTarget::~DAEChannelTarget(void)
{
}

daeElement* DAEChannelTarget::getTargetElement()
{
    if (!targetElement && targetId.length() > 0)
    {
        daeSIDResolver resolver(channel->getDocument()->getDomRoot(), targetId.c_str());
        targetElement = resolver.getElement();
    }
    return targetElement;
}

const std::string& DAEChannelTarget::getTargetId() const
{
    return targetId;
}

size_t DAEChannelTarget::getTargetAttributeCount() const
{
    return attributeIds.size();
}

daeElement* DAEChannelTarget::getTargetAttribute(size_t index)
{
    if (index >= attributeIds.size())
    {
        return NULL;
    }
    const std::string& att = attributeIds[index];
    std::string sid = att.substr(0, att.find('.'));
    daeSIDResolver resolver(getTargetElement(), sid.c_str());
    return resolver.getElement();
}

void DAEChannelTarget::getPropertyName(size_t index, std::string* str)
{
    if (index < attributeIds.size())
    {
        // The property name is the string segment after the '.'
        // The propery is optional so it might not be found.
        const std::string& att = attributeIds[index];
        size_t i = att.find('.');
        if (i != std::string::npos && i < att.size())
        {   
            str->assign(att.substr(i+1));
            return;
        }
    }
    str->clear();
}

}